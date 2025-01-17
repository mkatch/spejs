import * as THREE from 'three';
import { Matrix3, Matrix4, Vector3, Quaternion } from 'three';
import { decodeQoi, decodeQoiHeader } from './qoi';
import { TaskListenRequest, TaskRequest, TaskResponse, TaskScheduleRequest, TaskServiceClient } from '@gen/proto/task';
import { SkyboxRequest, SkyboxResponse } from '@gen/proto/skybox';

const canvasContainerElement = document.getElementById('canvas-container') as HTMLDivElement

const frontendUrl = `http://${window.location.hostname}:6101`;
const taskService = new TaskServiceClient(frontendUrl, undefined);

let time = 0

const TEST_CUBE_MAP: THREE.CubeTexture = (() => {
	const c = document.createElement('canvas').getContext('2d')!;
	const faceSize = 32;
	c.canvas.width = faceSize;
	c.canvas.height = faceSize;
	c.font = `bold ${faceSize * 0.5}px monospace`;
	c.textAlign = 'center';
	c.textBaseline = 'middle';
	const faces = [
		['#F00', 'px'],
		['#880', 'nx'],
		['#0F0', 'py'],
		['#088', 'ny'],
		['#00F', 'pz'],
		['#808', 'nz'],
	].map(([color, text]) => {
		c.fillStyle = color!;
		c.fillRect(0, 0, faceSize, faceSize);
		c.fillStyle = 'white';
		c.fillText(text!, faceSize / 2, faceSize / 2);
		return c.canvas.toDataURL();
	});
	return new THREE.CubeTextureLoader().load(faces)
})()

const env = createEnvMesh()

let skyboxAtlasBuffer: ArrayBuffer = new ArrayBuffer(0)

async function applySkyboxResponse(skybox: SkyboxResponse) {
	const assetUrl = `http://${window.location.hostname}:8000/static/${skybox.getPath()}`;
	const rsp = await fetch(assetUrl)
	const rspData = await rsp.arrayBuffer()

	const { width: atlasWidth, height: atlasHeight } = decodeQoiHeader(rspData)
	if (atlasHeight != 6 * atlasWidth) {
		throw new Error(`Invalid skybox atlas dimensions: ${atlasWidth} x ${atlasHeight}. Expected height to be 6 times the width.`)
	}
	if (skyboxAtlasBuffer.byteLength !== atlasWidth * atlasHeight * 4) {
		skyboxAtlasBuffer = new ArrayBuffer(atlasWidth * atlasHeight * 4)
		const images = new Array<THREE.DataTexture>(6)
		const imageByteSize = atlasWidth * atlasWidth * 4
		for (const [i, _] of images.entries()) {
			const imageData = new Uint8ClampedArray(skyboxAtlasBuffer, i * imageByteSize, imageByteSize)
			const image = new THREE.DataTexture(imageData, atlasWidth, atlasWidth, THREE.RGBAFormat)
			image.flipY = false
			images[i] = image
		}
		const texture = new THREE.CubeTexture(images)
		texture.generateMipmaps = true
		texture.flipY = true
		env.material.uniforms.envMap!.value = texture
	}

	decodeQoi(rspData, { outChannels: 4, outBuffer: skyboxAtlasBuffer, flipX: true })
	const texture = env.material.uniforms.envMap!.value as THREE.CubeTexture
	texture.needsUpdate = true
	for (const image of texture.images) {
		image.needsUpdate = true
	}

	env.material.needsUpdate = true
}

const taskStream = taskService.listen(new TaskListenRequest())
taskStream.on("data", (rsp) => {
	console.log("Received task response: ", rsp.toObject())
	switch (rsp.getVariantCase()) {
		case TaskResponse.VariantCase.SKYBOX:
			return applySkyboxResponse(rsp.getSkybox()!)
		case TaskResponse.VariantCase.VARIANT_NOT_SET:
			return console.error("Received empty task response")
	}
})
taskStream.on("end", () => {
	console.log("Task stream ended")
})
taskStream.on("error", (err) => {
	console.error("Task stream error", err)
})

const scene = new THREE.Scene();
for (const [axis, color] of [
	[new Vector3(1, 0, 0), "#F00"],
	[new Vector3(-1, 0, 0), "#880"],
	[new Vector3(0, 1, 0), "#0F0"],
	[new Vector3(0, -1, 0), "#088"],
	[new Vector3(0, 0, 1), "#00F"],
	[new Vector3(0, 0, -1), "#808"],
] as const) {
	const geometry = new THREE.BoxGeometry(0.03, 0.03, 0.03)
	const material = new THREE.MeshBasicMaterial({ color })
	const mesh = new THREE.Mesh(geometry, material)
	mesh.position.copy(axis)
	scene.add(mesh)
}

scene.add(env)

let canvasWidth = NaN
let canvasHeight = NaN
const renderer = new THREE.WebGLRenderer({ antialias: true });
const camera = new THREE.PerspectiveCamera(70, 1, 0.01, 10);
camera.lookAt(0, 0, -1)
camera.up.set(0, 1, 0)
camera.updateMatrix()
const nearPyramid = new Matrix3()

let drag: {
	startQuaternion: Quaternion,
	startRay: Vector3,
	prevRay: Vector3,
	prevTime: number,
	endRay: Vector3,
	endTime: number,
} | undefined = undefined
let dragMomentum: {
	axis: Vector3,
	startVelocity: number,
	startQuaternion: Quaternion,
	startTime: number,
} | undefined = undefined
const onPointer = (e: PointerEvent) => {
	if (e.buttons & 1) {
		dragMomentum = undefined
		const ray = new Vector3(
			2 * e.offsetX / canvasHeight - 1,
			1 - 2 * e.offsetY / canvasHeight,
			1 / Math.tan(camera.fov / 360 * Math.PI)
		)
		if (!drag) {
			drag = {
				startQuaternion: camera.quaternion.clone(),
				startRay: ray,
				prevRay: ray,
				prevTime: time,
				endRay: ray,
				endTime: time,
			}
		} else {
			if (time - drag.endTime > 0.005) {
				drag.prevRay = drag.endRay
				drag.prevTime = drag.endTime
			}
			drag.endRay = ray
			drag.endTime = time
		}
	}

	if (drag) {
		const axis = new Vector3().crossVectors(drag.startRay, drag.endRay).normalize()
		if (Math.abs((axis.length() - 1)) < 1e-3) {
			const angle = drag.startRay.angleTo(drag.endRay)
			const deltaQuaternion = new Quaternion().setFromAxisAngle(axis, angle)
			camera.setRotationFromQuaternion(drag.startQuaternion.clone().multiply(deltaQuaternion))
		}
	}

	if (drag && !(e.buttons & 1)) {
		const lastAngle = drag.prevRay.angleTo(drag.endRay)
		const angularVelocity = Math.max(-6, Math.min(6, lastAngle / (drag.endTime - drag.prevTime)))
		const axis = new Vector3().crossVectors(drag.prevRay, drag.endRay).normalize()
		if (Math.abs(angularVelocity) > 0.05 && Math.abs(axis.length() - 1) < 1e-3) {
			dragMomentum = {
				startQuaternion: camera.quaternion.clone(),
				axis: new Vector3().crossVectors(drag.prevRay, drag.endRay).normalize(),
				startVelocity: angularVelocity,
				startTime: time,
			}
		} else {
			dragMomentum = undefined
		}
		drag = undefined
	}
}

for (const type of ['pointerdown', 'pointermove', 'pointerup'] as const) {
	renderer.domElement.addEventListener(type, onPointer)
}
renderer.domElement.addEventListener("dblclick", async () => {
	const step = camera.getWorldDirection(new Vector3()).multiplyScalar(2)
	camera.position.add(step)
	camera.updateMatrix()
	console.log(camera.position)
	scheduleSkybox()
})

function onCanvasContainerResize() {
	const width = Math.max(64, canvasContainerElement.offsetWidth)
	const height = Math.max(64, canvasContainerElement.offsetHeight)
	if (width === canvasWidth && height === canvasHeight) {
		return
	}

	canvasWidth = width
	canvasHeight = height
	renderer.setSize(width, height)
	camera.aspect = width / height
	camera.updateProjectionMatrix()

	nearPyramid.set(
		camera.aspect, 0, 0,
		0, 1, 0,
		0, 0, 1 / Math.tan(camera.fov / 360 * Math.PI),
	)

	draw()
}
onCanvasContainerResize()
new ResizeObserver(onCanvasContainerResize).observe(canvasContainerElement)

async function scheduleSkybox(): Promise<void> {
	const { x, y, z } = camera.position
	const skyboxRequest = new SkyboxRequest().setPositionList([x, y, z])
	const scheduleRequest = new TaskScheduleRequest().setRequest(new TaskRequest().setSkybox(skyboxRequest))
	const scheduleResponse = await taskService.schedule(scheduleRequest)
	console.log(scheduleResponse)
}
scheduleSkybox()

function draw() {
	if (dragMomentum) {
		if (dragMomentum.startTime === undefined) {
			dragMomentum.startTime = time
			dragMomentum.startQuaternion = camera.quaternion.clone()
		}

		const accel = -4 * Math.sign(dragMomentum.startVelocity)
		const maxDuration = -dragMomentum.startVelocity / accel
		const dt = Math.min(maxDuration, time - dragMomentum.startTime)

		const angle = dt * dragMomentum.startVelocity + 0.5 * accel * dt ** 2
		const deltaQuaternion = new Quaternion().setFromAxisAngle(dragMomentum.axis, angle)
		camera.setRotationFromQuaternion(dragMomentum.startQuaternion.clone().multiply(deltaQuaternion))

		if (dt >= maxDuration) {
			dragMomentum = undefined
		}
	}

	if (env) {
		const envCamera = env.material.uniforms.camera!.value as Matrix3
		envCamera
			.setFromMatrix4(new Matrix4().makeRotationFromQuaternion(camera.quaternion))
			// .transpose()
			.multiply(nearPyramid)
	}

	renderer.render(scene, camera)
}

renderer.setAnimationLoop((t) => {
	time = t / 1000
	draw()
})

canvasContainerElement.appendChild(renderer.domElement);

function createEnvMesh(): THREE.Mesh<THREE.BufferGeometry, THREE.RawShaderMaterial> {
	const geometry = new THREE.BufferGeometry()
	geometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array([
		-1, -1, 1, 1, -1, 1,
		-1, -1, 1, -1, 1, 1,
	]), 2));
	// Set bounding sphere to avoid it being computed fro position, which would
	// fail. Otherwise unused.
	geometry.boundingSphere = new THREE.Sphere();

	const material = new THREE.RawShaderMaterial({
		uniforms: {
			camera: { value: new Matrix3() },
			envMap: { value: null },
			axisMap: { value: TEST_CUBE_MAP },
		},
		vertexShader: `
      uniform mat3 camera;
      attribute vec2 position;
      varying vec3 envCoords;
      void main() {
        envCoords = camera * vec3(position, -1.0);
        gl_Position = vec4(position, 0.0, 1.0);
      }
  `,
		fragmentShader: `
      uniform samplerCube envMap;
      uniform samplerCube axisMap;
      varying highp vec3 envCoords;
      void main() {
        gl_FragColor =
          1.0 * textureCube(envMap, envCoords) +
          0.0 * textureCube(axisMap, envCoords);
      }
  `,
		depthTest: false,
		depthWrite: false,
	});
	const mesh = new THREE.Mesh(geometry, material);
	mesh.frustumCulled = false;
	mesh.renderOrder = -1;
	return mesh;
}
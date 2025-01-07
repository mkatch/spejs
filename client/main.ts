import * as THREE from 'three';
import { Matrix3, Matrix4, Vector2, Vector3, Quaternion } from 'three';
import { qoiDecode } from './qoi';
import { SkyboxServiceClient } from '@proto/SkyboxServiceClientPb';
import { TaskServiceClient } from '@proto/TaskServiceClientPb';
import { SkyboxRenderRequest, SkyboxRenderResult } from '@proto/skybox_pb';
import { TaskPollRequest, TaskResult } from '@proto/task_pb';
import { max } from 'three/examples/jsm/nodes/Nodes.js';

const canvasContainerElement = document.getElementById('canvas-container') as HTMLDivElement

const frontendUrl = `http://${window.location.hostname}:6101`;
const skyboxService = new SkyboxServiceClient(frontendUrl, undefined);
const taskService = new TaskServiceClient(frontendUrl, undefined);

const skyboxRenderRequest = new SkyboxRenderRequest();
skyboxRenderRequest.setPositionList([0, 0, 0]);
void skyboxService.render(skyboxRenderRequest)

let time = 0

let env: THREE.Mesh<THREE.BufferGeometry, THREE.RawShaderMaterial> | undefined //= createEnvMesh();

async function applySkyboxRenderResult(result: SkyboxRenderResult) {
	console.log("Applying skybox render result: ", result.toObject())

	const assetUrl = `http://${window.location.hostname}:8000/static/${result.getPath()}`;
	const frsp = await fetch(assetUrl)
	const d = await frsp.arrayBuffer()
	const q = qoiDecode(d, { outputChannels: 4 })
	console.log("skybox", q.width, 'x', q.height)
	const images = new Array<THREE.DataTexture>(6)
	const ii = new Uint8Array(q.data)
	for (let i = 0; i < images.length; ++i) {
		const s = q.width * q.width * 4
		const ab = new Uint8Array(s)

		for (let r = 0; r < q.width; ++r) {
			const i0 = (i + 1) * s - (r + 1) * q.width * 4
			const o0 = r * q.width * 4
			for (let c = 0; c < q.width * 4; ++c) {
				ab[o0 + c] = ii[i0 + c]
			}
		}
		const t = new THREE.DataTexture(ab, q.width, q.width, THREE.RGBAFormat)
		t.generateMipmaps = true
		t.needsUpdate = true
		images[i] = t
	}
	const envMap = new THREE.CubeTexture(images)
	envMap.needsUpdate = true
	env = createEnvMesh(envMap)
	scene.add(env)
}

async function pollTasks() {
	try {
		console.log("Polling tasks...")
		const req = new TaskPollRequest()
		const rsp = await taskService.poll(req)
		for (const result of rsp.getResultsList()) {
			switch (result.getResultCase()) {
				case TaskResult.ResultCase.SKYBOX_RENDER:
					applySkyboxRenderResult(result.getSkyboxRender()!)
					break
				default:
					console.error("Unknown result type: ", result.getResultCase())
			}
		}
	} catch (e) {
		console.log(e);
	}

	finally {
		setTimeout(pollTasks, 1000)
	}
}
setTimeout(pollTasks, 1000)

const scene = new THREE.Scene();

let canvasWidth = NaN
let canvasHeight = NaN
const renderer = new THREE.WebGLRenderer({ antialias: true });
const camera = new THREE.PerspectiveCamera(70, 1, 0.01, 10);
camera.position.z = 1;
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
		const angle = drag.startRay.angleTo(drag.endRay)
		const deltaQuaternion = new Quaternion().setFromAxisAngle(axis, -angle)
		camera.setRotationFromQuaternion(drag.startQuaternion.clone().multiply(deltaQuaternion))
	}

	if (drag && !(e.buttons & 1)) {
		const lastAngle = drag.prevRay.angleTo(drag.endRay)
		let angularVelocity = lastAngle / (drag.endTime - drag.prevTime)
		if (Math.abs(angularVelocity) < 0.05) {
			dragMomentum = undefined
		} else {
			if (Math.abs(angularVelocity) > 6) {
				angularVelocity = 6 * Math.sign(angularVelocity)
			}
			dragMomentum = {
				startQuaternion: camera.quaternion.clone(),
				axis: new Vector3().crossVectors(drag.prevRay, drag.endRay).normalize(),
				startVelocity: angularVelocity,
				startTime: time,
			}
		}
		drag = undefined
	}
}
for (const type of ['pointerdown', 'pointermove', 'pointerup'] as const) {
	renderer.domElement.addEventListener(type, onPointer)
}

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
		const deltaQuaternion = new Quaternion().setFromAxisAngle(dragMomentum.axis, -angle)
		camera.setRotationFromQuaternion(dragMomentum.startQuaternion.clone().multiply(deltaQuaternion))

		if (dt >= maxDuration) {
			dragMomentum = undefined
		}
	}

	if (env) {
		const envCamera = env.material.uniforms.camera!.value as Matrix3
		envCamera
			.setFromMatrix4(new Matrix4().makeRotationFromQuaternion(camera.quaternion))
			.multiply(nearPyramid)
	}

	renderer.render(scene, camera)
}

renderer.setAnimationLoop((t) => {
	time = t / 1000
	draw()
})

canvasContainerElement.appendChild(renderer.domElement);


const TEST_CUBE_MAP: THREE.CubeTexture = (() => {
	const c = document.createElement('canvas').getContext('2d')!;
	const faceSize = 32;
	c.canvas.width = faceSize;
	c.canvas.height = faceSize;
	c.font = `bold ${faceSize * 0.5}px monospace`;
	c.textAlign = 'center';
	c.textBaseline = 'middle';
	const faces = [
		['#F00', '+X'],
		['#F80', '-X'],
		['#0A0', '+Y'],
		['#8A0', '-Y'],
		['#00F', '+Z'],
		['#80F', '-Z'],
	].map(([color, text]) => {
		c.fillStyle = color!;
		c.fillRect(0, 0, faceSize, faceSize);
		c.fillStyle = 'white';
		c.fillText(text!, faceSize / 2, faceSize / 2);
		return c.canvas.toDataURL();
	});
	return new THREE.CubeTextureLoader().load(faces)
})();

function createEnvMesh(envMap: THREE.CubeTexture): THREE.Mesh<THREE.BufferGeometry, THREE.RawShaderMaterial> {
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
			envMap: { value: envMap },
			axisMap: { value: TEST_CUBE_MAP },
		},
		vertexShader: `
      uniform mat3 camera;
      attribute vec2 position;
      varying vec3 envCoords;
      void main() {
        envCoords = camera * vec3(position, 1.0);
        gl_Position = vec4(position, 0.0, 1.0);
      }
  `,
		fragmentShader: `
      uniform samplerCube envMap;
      uniform samplerCube axisMap;
      varying highp vec3 envCoords;
      void main() {
        gl_FragColor =
          0.9 * textureCube(envMap, envCoords) +
          0.1 * textureCube(axisMap, envCoords);
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
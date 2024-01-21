import * as foo from './foo';
import * as empty_pb from 'proto/empty_pb';
import { JobServiceClient } from 'proto/JobServiceClientPb';
import * as universe_pb from 'proto/universe_pb';
import { UniverseServiceClient } from 'proto/UniverseServiceClientPb';
import * as THREE from 'three';

let name: string = 'World';
foo.sayHello(name);
const client = new JobServiceClient('/rpc')
client.status(new empty_pb.Empty(), {}, (err, resp) => {
  console.log("err: ", err)
  console.log("resp: ", resp.toObject())
});
console.log(client)
console.log(empty_pb.Empty)

function main() {
  document.body.innerHTML = `
      <h1>Spejs</h1>
  
      <label for="coords-input">Coordinates:</label>
      <input type="text" id="coords-input">
  
      <button id="look-button">Look</button>
    `;

  const coordsInput = document.getElementById('coords-input') as HTMLInputElement;
  const lookButton = document.getElementById('look-button') as HTMLButtonElement;

  const universeService = new UniverseServiceClient('/rpc');

  lookButton.addEventListener('click', async () => {
    const coords = coordsInput.value.split(',').map(s => parseInt(s));
    if (coords.length !== 2 || isNaN(coords[0]!) || isNaN(coords[1]!)) {
      console.error(`Invalid coodrinate input "${coordsInput.value}"`);
      return;
    }

    const request = new universe_pb.OpticalSampleRequest();
    request.setX(coords[0]!);
    request.setY(coords[1]!);
    try {
      const response = await universeService.opticalSample(request);
      console.log(response.toObject());
    } catch (e) {
      console.error('===ERROR: ', e);
    }
  });
  console.log("hello");

  const width = 500, height = 500;
  const camera = new THREE.PerspectiveCamera(70, width / height, 0.01, 10);
  camera.position.z = 1;

  const scene = new THREE.Scene();

  const geometry = new THREE.SphereGeometry(-0.5, 20, 10);
  const material = new THREE.MeshNormalMaterial();
  const wireframe = new THREE.WireframeGeometry(geometry);
  const wireframeMaterial = new THREE.LineBasicMaterial({ color: 0x000000 });

  const mesh = new THREE.Mesh(geometry, material);
  const w = new THREE.LineSegments(wireframe, wireframeMaterial);
  w.scale.multiplyScalar(0.999);
  scene.add(mesh, w);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(width, height);
  renderer.setAnimationLoop(time => {
    mesh.rotation.x = time / 2000;
    mesh.rotation.y = time / 1000;
    w.rotation.copy(mesh.rotation);

    renderer.render(scene, camera);
  });
  document.body.appendChild(renderer.domElement);
}

main();
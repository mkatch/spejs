import * as foo from './foo';
import * as empty_pb from 'proto/empty_pb';
import { JobServiceClient } from 'proto/JobServiceClientPb';
import * as universe_pb from 'proto/universe_pb';
import { UniverseServiceClient } from 'proto/UniverseServiceClientPb';

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
    
    <canvas id="canvad" width="100" height="100"></canvas>
  `;

  const coordsInput = document.getElementById('coords-input') as HTMLInputElement;
  const lookButton = document.getElementById('look-button') as HTMLButtonElement;
  // const canvas = document.getElementById('canvas') as HTMLCanvasElement;

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
}

main();
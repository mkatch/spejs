import * as foo from './foo';
import * as empty_pb from 'proto/empty_pb';
import { JobServiceClient } from 'proto/JobServiceClientPb';
let name: string = 'World';
foo.sayHello(name);
const client = new JobServiceClient('/rpc')
client.status(new empty_pb.Empty(), {}, (err, resp) => {
  console.log("err: ", err)
  console.log("resp: ", resp.toObject())
});
console.log(client)
console.log(empty_pb.Empty)

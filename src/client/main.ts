import * as foo from './foo';
import * as empty_pb from 'proto/empty_pb';
import { JobServiceClient } from 'proto/JobServiceClientPb';
let name: string = 'World';
foo.sayHello(name);
const client = new JobServiceClient("hello")
console.log(client)
console.log(empty_pb.Empty)

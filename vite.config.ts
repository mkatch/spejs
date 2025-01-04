import { defineConfig, Plugin } from 'vite'
import tsconfigPaths from 'vite-tsconfig-paths'
import { resolve } from 'path'

const jobStatus: Plugin = {
	name: "job-status",
	configureServer(server) {
		server.middlewares.use("/job", (req, res) => {
			console.log(req.method, req.url)
			if (req.method === "GET" && req.url === "/attach.json") {
				res.setHeader("Content-Type", "application/json")
				res.write(JSON.stringify({
					command: process.argv.join(" "),
					pid: process.pid,
				}))
			} else if (req.method === "GET" && req.url === "/status.json") {
				res.setHeader("Content-Type", "application/json")
				res.write(JSON.stringify({ ready: true }))
			} else if (req.method === "POST" && req.url === "/quit") {
				setTimeout(() => server.close(), 0)
			} else {
				res.statusCode = 400
			}
			res.end()
		})
	}
}

export default defineConfig({
	build: {
		rollupOptions: {
			input: {
				client: resolve(__dirname, 'client/index.html'),
			}
		}
	},
	plugins: [
		tsconfigPaths(),
		jobStatus
	],
});
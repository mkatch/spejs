import { defineConfig } from 'vite'
import tsconfigPaths from 'vite-tsconfig-paths'
import { resolve } from 'path'

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
	],
});
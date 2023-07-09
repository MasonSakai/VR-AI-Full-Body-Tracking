//import { Worker } from 'worker_threads';

const camWorker = new Worker(new URL('./camera-worker.js', import.meta.url));
const aiWorker = new Worker(new URL('./ai-worker.js', import.meta.url));
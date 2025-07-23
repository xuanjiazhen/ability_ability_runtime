import { taskpool, worker } from "@kit.ArkTS";

@Concurrent
export function throwErrorTask() {
  console.info('========2')
  throw new Error("taskpool error");
}

@Concurrent
export function normalTask(): number {
  return 100;
}

@Concurrent
export function longErrorTask(): void {
  let t = Date.now();
  while ((Date.now() - t) < 100) {
  }
  throw new Error("long task error");
}

@Concurrent
export function longTaskWithData(num: number): number {
  let res: number = num * 10;
  taskpool.Task.sendData(res);
  return num;
}

@Concurrent
export async function AsyncTask(): Promise<string> {
  return "async1 done";
}

@Concurrent
export async function AsyncTaskWithError(): Promise<string> {
  throw new Error("taskpool error");
}



// @Concurrent
// export async function WorkerNormalAsyncTask(): Promise<string> {
//   return new Promise((resolve) => {
//     const myWorker = new worker.ThreadWorker('../workers/Worker6');
//     myWorker.onmessage = (e) => {
//       resolve(e.data);
//     };
//     myWorker.postMessage("start");
//   });
// }
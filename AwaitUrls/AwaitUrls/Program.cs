using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace AwaitUrls
{
    class Program
    {
        static async Task<string> WaitOnce() {
            await Task.Delay(1000);
            return "<waited>";
        }

        static async Task<string> GetUrl(string url) {
            HttpClient client = new HttpClient();
            Task<Stream> result = client.GetStreamAsync(url);
            Stream stream = await result;
            StreamReader reader = new StreamReader(stream);
            return await reader.ReadToEndAsync();
        }

        static async Task GetThingsSerial() {
            string html = await GetUrl("http://reddit.com");
            Console.WriteLine(html);
            Console.WriteLine(await GetUrl("http://www.slowinternet.com/"));
            Console.WriteLine(await GetUrl("http://google.com"));
            Console.WriteLine(await GetUrl("http://reddit.com/r/Jokes"));
            //Console.WriteLine(await GetUrl(html.Substring(0, 10)));
        }

        public static IEnumerable<Task<T>> Interleaved<T>(IEnumerable<Task<T>> tasks) {
            // https://blogs.msdn.microsoft.com/pfxteam/2012/08/02/processing-tasks-as-they-complete/
            var inputTasks = tasks.ToList();
            var buckets = new TaskCompletionSource<T>[inputTasks.Count];
            var results = new Task<T>[buckets.Length];
            for (int i = 0; i < buckets.Length; i++)
            {
                buckets[i] = new TaskCompletionSource<T>();
                results[i] = buckets[i].Task;
            }
            int nextTaskIndex = -1;
            Action<Task<T>> continuation = completed =>
            {
                var bucket = buckets[System.Threading.Interlocked.Increment(ref nextTaskIndex)];
                if (completed.IsFaulted) bucket.TrySetException(completed.Exception.InnerExceptions);
                else if (completed.IsCanceled) bucket.TrySetCanceled();
                else bucket.TrySetResult(completed.Result);
            };

            foreach (var inputTask in inputTasks)
                inputTask.ContinueWith(continuation, System.Threading.CancellationToken.None, TaskContinuationOptions.ExecuteSynchronously, TaskScheduler.Default);
            return results;
        }

        static async Task GetThingsParallel()
        {
            var tasks = new[] {
                GetUrl("http://reddit.com"),
                GetUrl("http://www.slowinternet.com/"),
                GetUrl("http://google.com"),
                GetUrl("http://reddit.com/r/Jokes")
            };

            // Wait for all
            //string[] results = await Task.WhenAll(tasks);
            //Console.WriteLine(string.Join("", results.Select(str => str.Substring(0, 10))));

            // First ready, first handled
            foreach (Task<string> strTask in Interleaved(tasks)) {
                string res = await strTask;
                Console.WriteLine(res.Substring(0, 25));
            }

            //var allAtOnce = Task.WhenAll(new List<Task<string>>() { GetUrl("http://www.example.com/"), WaitOnce(), WaitOnce(), WaitOnce() });
            //combined = string.Join("", await allAtOnce);
            //Console.WriteLine(combined);

            //string combined = await GetUrl("http://www.slowinternet.com/") + await GetUrl("http://google.com") + await WaitOnce();
            string combined = await WaitOnce();
            Console.WriteLine(combined);
            // slow
            combined = await WaitOnce() + await WaitOnce();
            Console.WriteLine(combined);
            //slowest
            combined = await WaitOnce() + await WaitOnce() + await WaitOnce() + await WaitOnce();
            Console.WriteLine(combined);

            // fast in parallel
            var allAtOnce = Task.WhenAll(new List<Task<string>>() { WaitOnce(), WaitOnce(), WaitOnce(), WaitOnce() });
            combined = string.Join("", await allAtOnce);
            Console.WriteLine(combined);
        }

        static void Main(string[] args) {
            GetThingsParallel().Wait();
        }
    }
}

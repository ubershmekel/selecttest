using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace ListenAwait
{
    class Program
    {
        static void Main(string[] args)
        {
            AcceptLoop().Wait();
        }

        public static async Task AcceptLoop()
        {
            int port = 1234;
            var host = Dns.GetHostEntry("localhost").AddressList
                .First(item=>item.AddressFamily == AddressFamily.InterNetwork);
            TcpListener listener = new TcpListener(host, port);
            listener.Start();
            Console.WriteLine("AcceptLoop is now running");
              
            Console.WriteLine(" on port " + port);
            while (true)
            {
                try
                {
                    Console.WriteLine("Awaiting");
                    TcpClient tcpClient = await listener.AcceptTcpClientAsync();

                    Task procTask = Process(tcpClient);
                    //await procTask;

                } catch (Exception ex) {
                    Console.WriteLine(ex.Message);
                }
            }
        }

        public static async Task Process(TcpClient tcpClient)
        {
            NetworkStream networkStream = tcpClient.GetStream();
            StreamReader reader = new StreamReader(networkStream);
            StreamWriter writer = new StreamWriter(networkStream);
            writer.AutoFlush = true;
            Console.WriteLine("ReadLineAsync");
            string request = await reader.ReadLineAsync();
            if (request != null)
            {
                Console.WriteLine("Received service request: " + request);
                string response = request.Length.ToString();
                Console.WriteLine("Computed response is: " + response + "\n");
                await writer.WriteAsync(response);
            }
            else {
                // Client closed connection
            }
            tcpClient.Close();
        }
    }
}

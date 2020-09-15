using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace HL_Client
{
    public class TCP_Server
    {
        #region Fields
        private int m_port;
        private IPAddress m_ip;
        private TcpListener m_server;
        #endregion


        public TCP_Server(int port)
        {
            m_port = port;
            m_ip = IPAddress.Parse("127.0.0.1");
            m_server = new TcpListener(m_ip, m_port);
        }

        ~TCP_Server()
        {
            m_port = 0;
            m_ip = null;
            m_server = null;

        }

        public void listen()
        {
            m_server.Start();

            // Buffer for reading data
            Byte[] bytes = new Byte[256];
            String data = null;

            // Enter the listening loop.
            while (true)
            {
                Console.Write("Waiting for a connection... ");

                // Perform a blocking call to accept requests.
                // You could also use server.AcceptSocket() here.
                TcpClient client = m_server.AcceptTcpClient();
                Console.WriteLine("Connected!");

                data = null;

                // Get a stream object for reading and writing
                NetworkStream stream = client.GetStream();

                int i;

                // Loop to receive all the data sent by the client.
                while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                {
                    // Translate data bytes to a ASCII string.
                    data = System.Text.Encoding.ASCII.GetString(bytes, 0, i);
                    Console.WriteLine("Received: {0}", data);

                    // Process the data sent by the client.
                    data = data.ToUpper();

                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(data);

                    // Send back a response.
                    stream.Write(msg, 0, msg.Length);
                    Console.WriteLine("Sent: {0}", data);
                }

                // Shutdown and end connection
                client.Close();
            }
        }

        void write(byte[] data)
        {


        }

        byte[] receive()
        {
            byte[] tmp = new byte[4];

            return tmp;
        }
      
    }
}

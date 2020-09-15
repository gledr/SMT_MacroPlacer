using System;

namespace HL_Client
{
    class Program
    {
        static void Main(string[] args)
        {
            TCP_Server server = new TCP_Server(1111);
            server.listen();
        }
    }
}

//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : Heuristics Lab Client
// Workfile     : Program.cs
//
// Date         : 01. October 2020
// Compiler     : .NET Framework 4.6.1
// Copyright    : Johannes Kepler University
// Description  : Heuristics Lab Backend
//==================================================================

namespace HL_Client
{
    class Program
    {
        static void Main(string[] args)
        {
            const int port = 1111;

            System.Console.WriteLine("SMT_MacroPlacer: Heuristics Lab Fronend");
            System.Console.WriteLine("Handle Placement Problems transmitted by TCP/IP");
            System.Console.WriteLine("");
            System.Console.WriteLine("Copyright: Johannes Kepler University Linz");
            System.Console.WriteLine("Author   : Pointner Sebastian");
            System.Console.WriteLine("Version  : 0.1");
            System.Console.WriteLine("");

            TCP_Server server = new TCP_Server(port);
            server.handle_requests();

            System.Console.WriteLine("");
            System.Console.WriteLine("");
            System.Console.WriteLine("Heuristics Lab Fronend Terminating");
        }
    }
}

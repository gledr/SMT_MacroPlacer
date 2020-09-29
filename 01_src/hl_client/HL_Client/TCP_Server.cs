using System;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace HL_Client
{
    public class TCP_Server
    {
        #region Fields
        private readonly string INIT_TAG = "#**#INIT#**#";
        private readonly string SET_PROBLEM_TAG = "#**#SETPROBLEM#**#";
        private readonly string SOLVE_PROBLEM_TAG = "#**#SOLVEPROBLEM#**#";
        private readonly string GET_SOLUTION_TAG = "#**#GETSOLUTION#**#";
        private readonly string TERMINATE_SERVER_TAG = "#**#TERMINATESERVER#**#";

        private int m_port;
        private TcpListener m_server;
        private TcpClient m_client;
        private NetworkStream m_stream;
        private enum BackendMode { eInit, eSetProblem, eSolve, eGetSolution, eTerminate };
        private BackendMode m_state;

        #endregion


        public TCP_Server(int port)
        {
            m_port = port;
            m_server = new TcpListener(IPAddress.Any, m_port);
            m_state = BackendMode.eInit;
        }

        ~TCP_Server()
        {
            m_port = 0;
            m_server = null;

        }

        public void listen()
        {
            m_server.Start();
            Console.Write("Waiting for a connection... ");
            m_client = m_server.AcceptTcpClient();
            Console.WriteLine("Connected!");
            m_stream = m_client.GetStream();

            bool done = false;

            while (true)
            {
                switch (m_state)
                {
                    case BackendMode.eInit:
                        // Chill dude
                        m_state = this.next_state();
                        break;

                    case BackendMode.eSetProblem:
                        string msg = this.receive(); 
                        System.Console.WriteLine(msg);
                        m_state = this.next_state();

                        break;

                    case BackendMode.eSolve:

                        m_state = this.next_state();
                        break;

                    case BackendMode.eGetSolution:

                        m_state = this.next_state();
                        break;

                    case BackendMode.eTerminate:
                        this.terminate();
                        done = true;
                        break;

                    default:
                        throw new NotImplementedException();
                }

                if (done)
                {
                    break;
                }
            }
        }

        BackendMode next_state()
        {
            string msg = this.receive();      
            BackendMode ret_val = BackendMode.eInit;

            if (msg == INIT_TAG)
            {
                System.Console.WriteLine("Init State");
                ret_val = BackendMode.eInit;
            }
            else if (msg == SET_PROBLEM_TAG)
            {
                System.Console.WriteLine("Set Problem State");
                ret_val = BackendMode.eSetProblem;
            }
            else if (msg == SOLVE_PROBLEM_TAG)
            {
                System.Console.WriteLine("Solve Problem State");
                ret_val = BackendMode.eSolve;
            }
            else if (msg == GET_SOLUTION_TAG)
            {
                System.Console.WriteLine("Get Solution State");
                ret_val = BackendMode.eGetSolution;
            }
            else if (msg == TERMINATE_SERVER_TAG)
            {
                System.Console.WriteLine("Terminate State");
                ret_val = BackendMode.eTerminate;
            }
            else
            {
                throw new NotImplementedException();
            }

            return ret_val;
        }

        string add_delimiter(string msg)
        {
            return msg + "\n";
        }

        string strip_delimiter(string msg)
        {
            return msg.Substring(0, msg.Length - 1);
        }

        void terminate()
        {
            m_client.Close();
        }

        void set_problem(byte[] data)
        {

        }

        void write(string msg)
        {
            byte[] data = Encoding.ASCII.GetBytes(this.add_delimiter(msg));
            m_stream.Write(data, 0, data.Length);
        }

       string receive()
        {
            Byte[] bytes = new Byte[256];
            int len = m_stream.Read(bytes, 0, bytes.Length);
            String tmp = System.Text.Encoding.ASCII.GetString(bytes, 0, len);

            return this.strip_delimiter(tmp);
        }
      
    }
}

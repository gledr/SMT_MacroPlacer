using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using HeuristicLab.Algorithms.GeneticAlgorithm;
using HeuristicLab.Problems.LayoutOptimization;
using HeuristicLab.Data;
using HeuristicLab.ParallelEngine;
using Google.Protobuf;
using PlacerProto;
using System.Data.SqlTypes;
using System.Collections.Generic;
using System.Runtime.InteropServices;

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
        private readonly string[] DELIMITER = { "#\n#"};
        private readonly uint DELIIMITER_DIGITS = 3;

        private int m_port;
        private TcpListener m_server;
        private TcpClient m_client;
        private NetworkStream m_stream;
        private enum BackendMode { eInit, eSetProblem, eSolve, eGetSolution, eTerminate };
        private BackendMode m_state;
        private List<byte[]> m_traffic_in;
        private int m_pc;
        PlacerProto.MacroCircuit m_macro_circuit;
        #endregion


        public TCP_Server(int port)
        {
            m_port = port;
            m_server = new TcpListener(IPAddress.Any, m_port);
            m_state = BackendMode.eInit;
            m_traffic_in = new List<byte[]>();
            m_pc = 0;
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
                        byte[] msg = this.receive();
                        this.set_problem(msg);
                        m_state = this.next_state();
                        break;

                    case BackendMode.eSolve:
                        this.solve();
                        m_state = this.next_state();
                        break;

                    case BackendMode.eGetSolution:
                        this.get_solution();
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
            byte[] data = this.receive();
            string msg = System.Text.Encoding.ASCII.GetString(data);
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
                System.Console.WriteLine(msg);
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
            m_macro_circuit = MacroCircuit.Parser.ParseFrom(data);
        }

        void solve()
        {
            System.Console.WriteLine("Solve");
            
            GeneticAlgorithm algo = new GeneticAlgorithm();
            LayoutOptimizationProblem prob = new LayoutOptimizationProblem();
            
            prob.Macros = new IntMatrix(new[,] {
                { 400, 200 },
                { 600, 400 },
                { 300, 100 }
            });

            algo.Problem = prob;
            algo.MaximumGenerations.Value = 10;
            algo.Engine = new ParallelEngine();

            algo.Prepare();
            algo.Start();

            foreach (var res in algo.Results)
            {
                Console.WriteLine("{0} : {1}", res.Name, res.Value); 
            }

            
        }

        void get_solution()
        {
            System.Console.WriteLine("Get Solution");
            this.write("ACK#\n#");
        }

        void write(string msg)
        {
            System.Console.WriteLine("Answer {0}", msg);
            byte[] data = Encoding.ASCII.GetBytes(this.add_delimiter(msg));
            m_stream.Write(data, 0, data.Length);
        }

        byte[] receive()
        {
            // Still Data Availle in the Buffer
            if ((m_traffic_in.Count > 0) &&  (m_pc < m_traffic_in.Count))
            {
                int pos = m_pc;
                m_pc++;
                return m_traffic_in[pos];
            // Check Socket for Data
            } else
            {
                Byte[] bytes = new Byte[2048];
                int len = m_stream.Read(bytes, 0, bytes.Length);
                Byte[] bytes_cut = new Byte[len];
                Array.Copy(bytes, bytes_cut, len);

                byte[] pattern = new byte[] {35, 10, 35 };

                List<int> index = new List<int>();

                for (int i = 0; i < len-2; ++i)
                {
                    if (bytes_cut[i] == pattern[0] && bytes_cut[i+1] == pattern[1] && bytes_cut[i+2] == pattern[2])
                    {
                        index.Add(i);
                    }
                }
                
                List<byte[]> data = new List<byte[]>();
                for (int i = 0; i < index.Count; ++i)
                {
                    if (i == 0)
                    {
                        int frame_len = index[0];
                        Byte[] tmp = new Byte[frame_len];
                        Array.Copy(bytes_cut, tmp, frame_len);
                        data.Add(tmp);
                    }
                    else
                    {
                        int frame_len = index[i] - index[i-1] - 3;
                        Byte[] tmp = new Byte[frame_len];
                        
                        Array.Copy(bytes_cut, index[i-1]+3, tmp,0, frame_len);
                        data.Add(tmp);
                    }
                }

                for (int i = 0; i < data.Count; ++i)
                {
                    m_traffic_in.Add(data[i]);
                }
                int pos = m_pc;
                m_pc++;
                return m_traffic_in[pos];
            }
        }
    }
}

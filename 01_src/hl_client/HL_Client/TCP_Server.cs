//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : Heuristics Lab Client
// Workfile     : TCP_Server.cs
//
// Date         : 01. October 2020
// Compiler     : .NET Framework 4.6.1
// Copyright    : Johannes Kepler University
// Description  : TCP/IP Server handling requests
//==================================================================
using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using HeuristicLab.Algorithms.GeneticAlgorithm;
using HeuristicLab.Problems.LayoutOptimization;
using HeuristicLab.Data;
using HeuristicLab.ParallelEngine;
using PlacerProto;
using System.Collections.Generic;

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

        #region Constructor Destructor
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="port"></param>
        public TCP_Server(int port)
        {
            m_port = port;
            m_server = new TcpListener(IPAddress.Any, m_port);
            m_state = BackendMode.eInit;
            m_traffic_in = new List<byte[]>();
            m_pc = 0;
        }
        /// <summary>
        /// Destructor
        /// </summary>
        ~TCP_Server()
        {
            m_port = 0;
            m_server = null;
        }
        #endregion

        #region Public Methods
        /// <summary>
        /// Handle Incoming Requests
        /// </summary>
        public void handle_requests()
        {
            m_server.Start();
            Console.Write("Waiting for SMT_MacroPlacer to connect ... ");
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
                        this.debug_state(BackendMode.eInit);
                        m_state = this.next_state();
                        break;

                    case BackendMode.eSetProblem:
                        this.debug_state(BackendMode.eSetProblem);
                        byte[] msg = this.receive();
                        this.set_problem(msg);
                        m_state = this.next_state();
                        break;

                    case BackendMode.eSolve:
                        this.debug_state(BackendMode.eSolve);
                        this.solve();
                        m_state = this.next_state();
                        break;

                    case BackendMode.eGetSolution:
                        this.debug_state(BackendMode.eGetSolution);
                        this.get_solution();
                        m_state = this.next_state();
                        break;

                    case BackendMode.eTerminate:
                        this.debug_state(BackendMode.eTerminate);
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
        #endregion

        #region Private Methods
        /// <summary>
        /// Resolve Next State for Next State Logic Automata
        /// </summary>
        /// <returns>
        /// Enumeration Type for Next State
        /// </returns>
        private BackendMode next_state()
        { 
            byte[] data = this.receive();
            string msg = System.Text.Encoding.ASCII.GetString(data);
            BackendMode ret_val = BackendMode.eInit;

            if (msg == INIT_TAG)
            {
                ret_val = BackendMode.eInit;
            }
            else if (msg == SET_PROBLEM_TAG)
            {
                ret_val = BackendMode.eSetProblem;
            }
            else if (msg == SOLVE_PROBLEM_TAG)
            {
                ret_val = BackendMode.eSolve;
            }
            else if (msg == GET_SOLUTION_TAG)
            {
                ret_val = BackendMode.eGetSolution;
            }
            else if (msg == TERMINATE_SERVER_TAG)
            {
                ret_val = BackendMode.eTerminate;
            }
            else
            {
                throw new NotImplementedException(msg);
            }
            return ret_val;
        }

        /// <summary>
        /// Add Delimiter to TCP/IP Stream to mark end of message
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        private string add_delimiter(string msg)
        {
            return msg + DELIMITER[0];
        }

        /// <summary>
        /// Strip Delimiter from TCP/IP Stream
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        private string strip_delimiter(string msg)
        {
            return msg.Substring(0, msg.Length - 1);
        }

        /// <summary>
        /// Terminate Server
        /// </summary>
        private void terminate()
        {
            m_client.Close();
        }

        /// <summary>
        /// Set Heuristis Lab Problem
        /// </summary>
        /// <param name="data"></param>
        private void set_problem(byte[] data)
        {
            m_macro_circuit = MacroCircuit.Parser.ParseFrom(data);
        }

        /// <summary>
        /// Invoke Heuristics Lab to Solve Problem
        /// </summary>
        private void solve()
        {
            System.Console.WriteLine("Invoking Heuristics Lab for Placement");
            System.Console.WriteLine("Utilizing GenericAlgorithm");

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
            System.Console.WriteLine("Heuristics Lab Started...");
            algo.Start();
            System.Console.WriteLine("Heuristics Lab Terminated :)");
        }

        /// <summary>
        /// Print Current Automata State to Console
        /// </summary>
        /// <param name="state"></param>
        /// <returns></returns>
        private void debug_state(BackendMode state)
        {
            string ret_val = "State: ";
            switch (state)
            {
                case BackendMode.eInit:
                    ret_val += "Init";
                     break;
                case BackendMode.eSetProblem:
                    ret_val += "SetProblem";
                    break;
                case BackendMode.eSolve:
                    ret_val += "SolveProblem";
                    break;
                case BackendMode.eGetSolution:
                    ret_val += "GetSolution";
                    break;
                case BackendMode.eTerminate:
                    ret_val += "Terminate";
                    break;
                default:
                    throw new NotImplementedException();
            }
            System.Console.WriteLine(ret_val);
        }

        /// <summary>
        /// Send Heuristics Lab Solution Serialized back to SMT MacroPlacer
        /// </summary>
        private void get_solution()
        {
            foreach (Macro m in m_macro_circuit.M)
            {
                m.Lx = 42;
                m.Ly = 42;
            }

            m_macro_circuit.L.Ux = 100;
            m_macro_circuit.L.Uy = 100;

            string serial = m_macro_circuit.ToString();

            this.write(serial);
        }

        /// <summary>
        /// Write Data over TCP/IP
        /// </summary>
        /// <param name="msg"></param>
        private void write(string msg)
        {
            string tmp = this.add_delimiter(msg);
            byte[] data = Encoding.ASCII.GetBytes(tmp);
            m_stream.Write(data, 0, data.Length);
        }

        /// <summary>
        /// Get Data from TCP/IP
        /// </summary>
        /// <returns></returns>
        private byte[] receive()
        {
            // Still Data Availible in the Buffer
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
    #endregion
}

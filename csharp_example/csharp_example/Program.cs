using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace csharp_example
{

    public class BouncerWrapperInterface
    {
        public const string bouncerdll = "3drepobouncerwrapper";
        private bool connected = false;

        internal BouncerWrapperInterface()
        {
        }

       
        internal bool connect()
        {
            return connected = repoConnect("");
        }


        [DllImport(bouncerdll)]
        private static extern bool repoConnect(
             string config);

    }

    class Program
    {
        static void Main(string[] args)
        {
            BouncerWrapperInterface wrapper = new BouncerWrapperInterface();
            wrapper.connect();
        }
    }
}



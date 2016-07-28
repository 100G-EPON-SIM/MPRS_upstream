Last updated Oct 27, 2008

===================================================
Included files
===================================================

data_path.h	    - instantiate different state machines and simulates interaction among them.
FSM_base.h          - includes the base class for all the state machines and  a number of constants used throughout the environment.
FSM_DD.h            - includes implementation of data detector state machine for both ONU and OLT.
FSM_FEC.h           - includes implementation of fec decoder. 
FSM_ID.h            - includes implementation of idle deletion state machine.
FSM_II.h            - includes implementation of idle insertion state machine.
FSM_MAC.h           - includes  MAC implementation.
FSM_misc.h          - includes implementations of 64B/66B encoder, 66B/64B decoder, scrambler and descrambler state machines and MAC Client.
FSM_MPCP.h          - includes implementation of MPCP control multiplexor state machine.
FSM_XGMII.h         - includes implementation of XGMIIstate machine. 
sim_config.h        - includes several configuration parameters for simulation setup.
sim_output.h        - includes simulation out put functions.
stats.h             - generates statistics from simulation results.
timing_main.cpp	    - includes the "main" function and drives the whole simulation.


_list.h		-implements required data structures and data types  for simulation.
_queue.h	-implements required data structures and data types  for simulation. 
_types.h	-implements required data structures and data types  for simulation. 
_util.h		-implements required data structures and data types  for simulation. 


===================================================
How to install and run model
===================================================
You need to have Visual C++ 2008 Express Edition or equivalent.  This software is freely available online from Microsoft at http://www.microsoft.com/express/vc/.  Once installed, simply create a new project and add all of the above files.  When compiled, run the PhyTiming.exe file that is created, and the results will be displayed in the command prompt window as well as in some .csv files that can be viewed with Excel.  

===================================================
How to interpret results
===================================================
If the model is run with the default settings, then two .csv output files will be created that are labeled with the current time and date stamp.  The INFO file contains basic information about the test that was run, and the OUT2 file has the results.  

The OUT2 file has columns for each state machine and rows for delay in byte times.  At the top of the file it shows the min delay, max delay, and maximum drift for downstream operation.  If you scroll down a bit, the upstream results will be shown in a similar fashion.

Each cell corresponds to a delay value and a state machine.  The value within this cell is the percentage of blocks that experienced this delay.  For example, if you see a value of 1 corresponding to a delay of 0, this means that no delay was experienced by any blocks.  If you see a value of 0.6 corresponding to a delay of 4 and a value of 0.4 corresponding to a delay of 36, this means that 60% of the blocks experienced a delay of 4 bytes and the remaining 40% of blocks experienced a delay of 36 bytes.  


===================================================
How to create new state machine
===================================================
All state machines are derived from the base class fsm_base_t.  When defined, each state machine is given a timestamp index, an input type and an output type.  The default behavior is that the output data is equal to the input data.  

template< int16s L, class in_t, class out_t = in_t > class fsm_base_t

Each block of data is marked with a timestamp as it enters and leaves the state machine to determine the delay and delay variation of the particular state machine.  Each state machine is given a unique timestamp index so that the results can properly be examined once the test is complete. These different input and output types include 36-bit vectors, 66-bit vectors, 72-bit vectors, and frames. 

There are two main methods that each state machine inherits,TransmitUnit() and ReceiveUnit().  These methods are invoked when data is either taken from or passed into the state machine.  Other methods,if needed, are locally defined in the state machine file.  There is also a boolean indicator called OutputReady that can be queried if the state machine will not have its output avaialable on every clock.  

The FSM_II.h file contains the idle insertion state machine and is the easiest file to look at and understand the structure of how a state machine can be created. 

The data_path.h file shows the entire data path and how the different state machines are connected.  When creating a new state machine, under most circumstances, it will be used as a replacement for another state machine, and so this file would not have to be modified if the inputs and outputs remain the same.  



===================================================
How to configure simulation environment
=================================================== 
The sim_config.h file has a number of options that the user can select.  These options are enabled or disabled by commenting out or uncommenting the various define statements.  The most interesting ones are described below:

#define SHOW_64B_PACKETS
Since MAC should accummulate the entire frame before checking FCS and passing the frame to MPCP, by definition, a frame's delay in MAC will be proportional to the frame's length. This is the expected result, however it masks the undesiread delay variability that maybe introduced by the PCS state machines.  To avoid this, the smulation allows collecting the statistics only for 64-byte packets (MPCPDUs). If SHOW_64B_PACKETS is defined, simulation will run with all apcket sizes, but the statistic will be collected only for 64 byte packets.  If this define is not included, data will be collected on all packets and the results will be displayed for all packet lengths. 

#define CHECK_DOWNSTREAM
#define CHECK_UPSTREAM
These options will check results in a specific direction, or both, if they are both included.

#define SPARSE_TRAFFIC
If this define is left in, there will be random time gap bitween two consecutive frames (i.e light load), otherwise MAC_CLIENT will generate back to back frames.

#define RESULT_1_OUTPUT_FILE
#define RESULT_1_OUTPUT_SCREEN
These options allow the user to select whether results are sent to a file, to the standard otuput, or both. RESULT_1_OUTPUT in current simulation environment outputs delay (in byte times) per individual state diagram (function) and per individual packet. 

#define RESULT_2_OUTPUT_FILE
#define RESULT_2_OUTPUT_SCREEN
These options allow the user to select whether results are sent to a file, to the standard otuput, or both. RESULT_2_OUTPUT in current simulation environment outputs delay histogram (if also SHOW_HISTOGRAM is defined) and summary (min delay, max delay, delay variability).  

#define WARNING_OUTPUT_FILE
#define WARNING_OUTPUT_SCREEN
These options allow the user to select whether warnings are sent to a file, to the standard otuput, or both.

#define STOP_ON_WARNING
If defined, the simulation will stop if a warning is received.  


The data_path.h file also has some configurations that the user may want to modify.  Of most interest is the PacketSize() function.  Here, you can define the distribution of packet sizes to be used.  The default settings have 25% of the frames be 64-bytes and the remaining frames are uniformly distributed from 65 - 2000 bytes.  Another option in this file is the TEST_FRAMES constant.  This constant determines how many frames are sent when the model is run.  The default value is 100,000 frames.  


The FSM_base.h file contains a number of constants used throughout the environment.  Most of these constants do not have to be changed, but the user could make modifications to them here.  





















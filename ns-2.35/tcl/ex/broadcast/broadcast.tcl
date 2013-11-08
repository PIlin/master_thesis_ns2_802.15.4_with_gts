# ======================================================================
# Simulation Parameters
# ======================================================================

set RXThresh 1.76149e-10 ;#compute with threshold.cc for distance 300m
set node_dist 		20   ;# distance between cars
set max_dist  		8000 ;# road length
set stopSim 		30   ;#simulation time
set startNoise 		0    ;#noise traffic start time
set stopNoise 		22   ;#noise traffic end time
set startCBR 		2    ;#start time for our CBR
set stopCBR 		22   ;#end time for our CBR
set numNOISES 		5    ;#number of noise cars
set pkt_size		200  ;#size of cbr pkt in B
set cbr_interval	0.3  ;#CBR interval in s


# ======================================================================
# Define options
# ======================================================================
set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     [expr $max_dist / $node_dist] ;# number of mobilenodes
set val(rp)     DSDV                       ;# routing protocol (not used: broadcast transmissions)

# ======================================================================
# Main Program
# ======================================================================

#default
set srcnode [expr $val(nn) - 1] 

#taking parameter from command line
if {[llength $argv]  > 0} {        
   	set numNOISES [lindex $argv 0]
        set srcnode [lindex $argv 1]
        
}
puts "noises car $numNOISES src node $srcnode";


# 802.11g parameters
Mac/802_11 set bandwidth_ 54.0Mb
Mac/802_11 set basicRate_ 10.0Mb
Mac/802_11 set dataRate_ 54.0Mb
Mac/802_11 set ShortRetryLimit_ 7             ;# retransmittions
Mac/802_11 set LongRetryLimit_  4             ;# retransmissions
Mac/802_11 set CWMin_         15
Mac/802_11 set CWMax_         1023
Mac/802_11 set SlotTime_      0.000009        ;# 9us
Mac/802_11 set CCATime_       0.000003        ;# 3us
Mac/802_11 set RxTxTurnaroundTime_       0.000002  ;# 2us
Mac/802_11 set SIFS_          0.000016        ;# 16us
Mac/802_11 set PreambleLength_        96      ;# 16 bit
Mac/802_11 set PLCPHeaderLength_      40      ;# 24 bits
Mac/802_11 set PLCPDataRate_  6.0e6           ;# 6Mbps
Mac/802_11 set MaxPropagationDelay_  0.0000005     ;# 0.5us

Phy/WirelessPhy set RXThresh_ $RXThresh



#Initialize Global Variables

set ns_		[new Simulator]

$ns_ use-newtrace
set tracefd     [open UDP_Noise.tr w]
$ns_ trace-all $tracefd

#create $val(nn) cars
proc create_nodes {} {
	
	global ns_ val node_dist max_dist topo node_ src_node dst_node src_pos pkt_size udp null game cbr_interval
	
	set chan_1_ [new $val(chan)]
	$ns_ node-config -adhocRouting $val(rp) \
		 -llType $val(ll) \
		 -macType $val(mac) \
		 -ifqType $val(ifq) \
		 -ifqLen $val(ifqlen) \
		 -antType $val(ant) \
		 -propType $val(prop) \
		 -phyType $val(netif) \
		 -channel $chan_1_ \
		 -topoInstance $topo \
		 -agentTrace OFF \
		 -routerTrace OFF \
		 -macTrace OFF \
		 -movementTrace OFF			
		
	for {set i 0} {$i < $val(nn) } {incr i} {
		set node_($i) [$ns_ node]	
		$node_($i) random-motion 0 ;# disable random motion
		$node_($i) set X_ [expr ($i * $node_dist) % $max_dist]
		$node_($i) set Y_ 2.0
		set udp($i) [new Agent/Broadcastbase]
		$node_($i) attach $udp($i) 250
		$udp($i) set fid_ $i
		set game($i) [new Application/BroadcastbaseApp]		
		$game($i) set bsize_ $pkt_size
		$game($i) set bmsg-interval_ $cbr_interval
		$game($i) attach-agent $udp($i)		
	}
	
}

#finish procedure
proc finish {} {
	global ns_ tracefd
        $ns_ flush-trace
        close $tracefd
}

#set up topography object
set topo       [new Topography]

$topo load_flatgrid 20000 20000


#create God
create-god [expr $val(nn)]



#create nodes
create_nodes

#setting up noise cars
if { $numNOISES > 0 } {
	#set noise_interval
	set noise_interval [expr $val(nn) / $numNOISES ]

        for {set i 1} {$i <= $numNOISES } {incr i} {
		set index [expr $i * $noise_interval]
		set willStart [expr $index % $val(nn)]
		puts "noise node $willStart"
		$ns_ at $startNoise  "$game($willStart) start" 
		$ns_ at $stopNoise  "$game($willStart) stop" 
		
	}

}

#starting our cbr
$ns_ at $startCBR "$game($srcnode) start "
$ns_ at $stopCBR "$game($srcnode) stop "

#print-trace command is needed for all nodes in order to print hops
#and slots information
for {set i 0} {$i <  $val(nn) } {incr i} {
	$ns_ at [expr $stopSim + 0]   "$game($i) print-trace" 
}


#reset nodes
for {set i 0} {$i <  $val(nn) } {incr i} {
    $ns_ at $stopSim "$node_($i) reset";
}


$ns_ at $stopSim "finish"

$ns_ at [expr $stopSim + 0.01] "$ns_ halt"

$ns_ run



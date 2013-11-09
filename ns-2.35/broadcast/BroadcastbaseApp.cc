#include "timer-handler.h"
#include "packet.h"
#include "app.h"
#include "address.h"
#include "ip.h"
#include "bbcast-packet.h"
#include "BroadcastbaseAgent.h"
#include "node.h"
#include "BroadcastbaseApp.h"
#include "random.h"

#include <math.h>
#include <mobilenode.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
/* ----- used for debug printing...just comment them or no!! ------ */
//#define DEBUG_BROADCAST 

/* set it for normal behaviour...else not for taking the first leader absolutely */
#define RESTART_BROADCAST 
/* ------------------------------------------------------------------*/

extern FILE * rangeFile ;
extern FILE * traceFile ;


/* BroadcastbaseApp OTcl linkage class */
static class BroadcastbaseAppClass : public TclClass {
public:
	BroadcastbaseAppClass() : TclClass("Application/BroadcastbaseApp") {}
	TclObject* create(int, const char*const*) 
	{
		return (new BroadcastbaseApp);
	}
} class_app_broadcastbase;

/* -- this is the expiration timer to manage the interval of cbr --*/
void MyBCastCbrTimer::expire(Event*)
{
	if (t_->running_) t_->cbr_broadcast();
}

/* -- this is the expiration timer called when randow cw timer expires --*/
void MyBCastCWTimer::expire(Event* e)
{	
	t_->rcv_pkt[p_].nhop=hops_+1;
	t_->rcv_pkt[p_].slot_waiten=slot_waiten_;
	
	if (!(t_->rcv_pkt[p_].value) ) { /* I haven't still received pkt from opposite direction... I have to send it! */
				
		//print: forward time src_addr pkt_id=flow_id
		fprintf (traceFile,"f %.9f %d %ld\n",Scheduler::instance().clock(),nodeid,p_);
		fflush(traceFile);
		fflush(stdout);
		t_->rcv_pkt[p_].value=true; /* I'm sending p_, so I won't send it again!! */
		t_->send_broadcast(p_, dir_);
		free(t_->rcv_pkt[p_].cwt); /* I don't need this timer any more time */
		
	}
	/* else I've already received this packet form opposite direction...I don't need to forward it! Do nothing! */
}

/* Constructor (also initialize instances of timers) */
BroadcastbaseApp::BroadcastbaseApp() :  cw_timer_(this,1,1,'c',1,1), cbr_timer_(this)
{	running_=1;
	bind("bmsg-interval_", &binterval); // interval for broadcast msg
	bind("bsize_", &bsize); //size of broadcast msg
	bind("propagate_", &propagate);
	binterval=0;
	rnd_time=0;
	propagate = 1;
	fmd=0; 	//current fmd
	bmd=0; 	//current bmd
	fmr=0; 	//current fmr
	bmr=0; 	//current bmr
	ifmd=0; //index for lmfd
	ibmd=0; //index for lbmd
	ifmr=0; //index for lmfr
	ibmr=0;	//index for lbmr
		
	for (int i=0; i<MAXWINDOW;i++) {
		lfmd[i]=0;
		lbmd[i]=0;
		lfmr[i]=100;
		lbmr[i]=100;			
	}
	
}

/* -- OTcl command interpreter --*/
int BroadcastbaseApp::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc == 3) 
	{ 
		if (strcmp(argv[1], "attach-agent") == 0) 
		{
			agent_ = (Agent*) TclObject::lookup(argv[2]);
			if (agent_ == 0) 
			{
				tcl.resultf("no such agent %s", argv[2]);
				return(TCL_ERROR);
			}
			agent_->attachApp(this);
			return(TCL_OK);
		} 
	}
	if (argc == 2) 
	{ 
		if (strcmp(argv[1], "send-broadcast") == 0) 
		{
			send_broadcast((long int)-1,'e');		
			return(TCL_OK);
		} 
		if (strcmp(argv[1], "cbr-broadcast") == 0) 
		{
			cbr_broadcast();		
			return(TCL_OK);
		} 
 		if (strcmp(argv[1], "print-trace") == 0) 
 		{
 			print_trace();		
 			return(TCL_OK);
		} 
	}
	return (Application::command(argc, argv));
}



/* --simulation of a cbr using broadcast pkt --*/
void BroadcastbaseApp::cbr_broadcast() {
	send_broadcast((long int)-1,'e');
	if(binterval > 0)  cbr_timer_.resched(binterval);
}

/* --sending of broadcast pkt --*/
void BroadcastbaseApp::send_broadcast(long int id, char direction)
{
  	double x,y,z;
	hdr_bbcast mh_buf;
	nsaddr_t t;
	t=((BroadcastbaseAgent*)agent_)->addr();
	MobileNode *test = (MobileNode *) (Node::get_node_by_address(t));

	/* setting of field in the pkt. I'm preparing to send! */
	test->getLoc(&x,&y,&z);	
	mh_buf.x_=x;
	mh_buf.y_=y;
	mh_buf.dir_=direction;
	
	BBCastData *data = new BBCastData();	
	
	mh_buf.fmd_=fmd;
	mh_buf.bmd_=bmd;
	if (id >=0) { //forward
		mh_buf.nhop_=rcv_pkt[id].nhop; 
		mh_buf.slot_waiten_=rcv_pkt[id].slot_waiten+rcv_pkt[id].slot_choosen;
	}
	else { //send
		mh_buf.nhop_=0;
		mh_buf.slot_waiten_=0;
	}
	data->setHeader(&mh_buf);
	((BroadcastbaseAgent*)agent_)->sendbroadcastmsg(bsize, data, id);
	#ifdef DEBUG_BROADCAST
	cout << "BroadcastMsg - SENT \t- Node "<<((BroadcastbaseAgent*)agent_)->addr()<< "  pkt "<<id<<"\n";
	fflush(stdout);	
	#endif
}

void BroadcastbaseApp::start()
{	
	cbr_broadcast();	
}

void BroadcastbaseApp::stop() {
 running_=0;		
}

		
void BroadcastbaseApp::print_trace()
{
   
   map <long int,data_packet>::iterator it;
   #ifdef DESTNODE
   	if (((BroadcastbaseAgent*)agent_)->addr() == DESTNODE) {
   #endif			
   for( it =  rcv_pkt.begin(); it !=  rcv_pkt.end(); it++ ) {
     	
	   //print: node pkt hops slots
	   fprintf(traceFile,"n %d %ld %d %ld\n",((BroadcastbaseAgent*)agent_)->addr(),(*it).first,rcv_pkt[(*it).first].nhop,rcv_pkt[(*it).first].slot_waiten); 
	   fflush(traceFile);
	 
   }
   #ifdef DESTNODE
	}
   #endif	
}

/* Schedule next data packet transmission time */
double BroadcastbaseApp::next_snd_time()
{    	
	return interval;
}

/* Receive message from underlying agent */
void BroadcastbaseApp::process_data_BroadcastMsg(char *pkt, hdr_ip* ih)
{
	/*--------- ESTIMATION RANGE relative side -----------*/	
	char direction;
	BBCastData *data = new BBCastData(pkt);
	//looking for my position
	nsaddr_t t;
	t=((BroadcastbaseAgent*)agent_)->addr();
	MobileNode *test = (MobileNode *) (Node::get_node_by_address(t));
	double mpx,mpy,mpz;
	test->getLoc(&mpx,&mpy,&mpz);
	//sender position
	double spx=data->g_x();
	double spy=data->g_y();
	//sender distance from me
	double d=sqrt(pow((mpx - spx), 2) + pow((mpy - spy), 2));
	
	//print: received time node pkt
	fprintf (traceFile,"r %.9f %d %ld\n",Scheduler::instance().clock(),((BroadcastbaseAgent*)agent_)->addr(),data->g_id());
	fflush(traceFile);	
	#ifdef DEBUG_BROADCAST
	int sender_id=(int)(spx/20); /* ATTENTION: statically computed!!! */
	cout << "BroadcastMsg - RECEIVED - Node "<<((BroadcastbaseAgent*)agent_)->addr() << " from node "<<sender_id<<" pkt: "<<data->g_id()<<"\n";
	fflush(stdout);
	#endif

	if (!propagate)
		return;
	
	/* control and setting of message propagation direction */
	if(data->g_dir()=='e'){ // I have received from source
		if (mpx>=spx)   /* propagation direction is front received from back! */		
			direction='f';
		else 
			direction='b';
	}
	else 	//received from an intermediary hop
		direction=data->g_dir();
	
	/* source pkt control */
	if (mpx>=spx){ //received from back
		
		set_parameters_back(data, d, mpx, spx);
	}	
	else{ //received from front
		
		set_parameters_front(data, d, mpx, spx);
	}

	/*--------- MESSAGE PROPAGATION relative side -----------*/	
	/* New packet */
	if (rcv_pkt.find(data->g_id())==rcv_pkt.end()) { 
		if ((spx-mpx)>=0) { //received from FRONT
			if (direction=='b')
				broadcast_procedure(d,direction,data->g_id(),spx-mpx,data->g_nhop(),data->g_slot_waiten());
			else 	/* received from FRONT with direction=FRONT...I won't send it! */
				return;	
		}
		else{ //received from BACK
			if (direction=='f')
				broadcast_procedure(d,direction,data->g_id(),mpx-spx,data->g_nhop(),data->g_slot_waiten());
			else 	/* received from BACK with direction=BACK....I won't send it! */
				return;	
		}
	}
	else {  //received a msg already heard in the past		
		
		/* received a msg from other side then sender (i.e. different direction) */
		if (direction != rcv_pkt[data->g_id()].direction) {
			#ifdef DEBUG_BROADCAST
			cout << "WARNING - RECEIVED - Node "<<((BroadcastbaseAgent*)agent_)->addr() << " from other side pkt "<<data->g_id()<<" \n";
			#endif
			return;
		}
		if ((spx-mpx)>=0) { //received from FRONT
			if (direction=='b'){
				#ifdef RESTART_BROADCAST
				restart_broadcast_procedure(d,direction,data->g_id(),spx-mpx,data->g_nhop(),data->g_slot_waiten());
				#endif
			}
			else{  /* received from FRONT with direction=FRONT. I won't send it! */
				rcv_pkt[data->g_id()].value=true;
				#ifdef DEBUG_BROADCAST
				const char* s=rcv_pkt[data->g_id()].value ? "true" : "false";
				cout << "BroadcastMsg - RECEIVED - Node "<<((BroadcastbaseAgent*)agent_)->addr() << " set pkt "<<data->g_id()<<"  value: "<<s<<"\n";
				fflush(stdout);
				#endif
				return;
			}	
		}
		else {  //received from BACK
			if (direction=='f'){
				#ifdef RESTART_BROADCAST
				restart_broadcast_procedure(d,direction,data->g_id(),mpx-spx,data->g_nhop(),data->g_slot_waiten());
				#endif
				}
				else{ 	/* received from BACK with direction=BACK. I won't send it! */
				rcv_pkt[data->g_id()].value=true;
				#ifdef DEBUG_BROADCAST
				const char* s=rcv_pkt[data->g_id()].value ? "true" : "false";
				cout << "BroadcastMsg - RECEIVED - Node "<<((BroadcastbaseAgent*)agent_)->addr() << " set pkt "<<data->g_id()<<" value: "<<s<<"\n";
				fflush(stdout);
				#endif
				return;
			}
		}
	}
}


/* choose a value into CW and wait */
void BroadcastbaseApp::broadcast_procedure(double distance,char direction,long int gid,double dix,int hops, long int slot_w){
	int cw; //used for value of contention window
	double MaxRange;
	if (direction=='b')	//setting of MaxRange
		MaxRange= fmr;	  
	else if(direction=='f')
		MaxRange= bmr;	  
	//I get contention window (in slot)
	cw = compute_contwnd(distance,MaxRange);	 
	//random slot from CW
	Random::seed_heuristically();	 		 
	int rslot=Random::integer(cw);
	double r=rslot* SLOT;
	#ifdef DEBUG_BROADCAST
	cout<<"BroadcastMsg - Node "<<((BroadcastbaseAgent*)agent_)->addr() <<" value choosen "<<rslot<<" inside CW which is "<<r<<" sec\n";
	#endif
	MyBCastCWTimer *cwt =new MyBCastCWTimer(this,gid,((BroadcastbaseAgent*)agent_)->addr(),direction,hops,slot_w);
	/* create a new data packet structure with value=false before waiting time */
	data_packet dp;
	dp.value=false;
	dp.slot_choosen=rslot;
	dp.slot_waiten=0;
	dp.nhop=0;		
	dp.direction=direction;
	dp.cwt=cwt;
	dp.distance=dix;
	
	/* Insert of a new pkt in the map with value=false (i will send it if don't change ) and his timer */
	pair<long int,data_packet> Enumerator(gid,dp);
	rcv_pkt.insert(Enumerator);
	cwt->resched(r); /* new timer with the random value computed by cw */
}

/* received a pkt for which  I'm already waiting on a timer */
void BroadcastbaseApp::restart_broadcast_procedure(double distance,char direction,long int gid, double dix,int hops, long int slot_w){
	MyBCastCWTimer* timer=rcv_pkt[gid].cwt;
	
	/* received a pkt that i have already received in front of me in its direction, or i've already sent it */
	if (rcv_pkt[gid].value) {  
		#ifdef DEBUG_BROADCAST
		   cout<<"BroadcastMsg - DROP - Node "<<((BroadcastbaseAgent*)agent_)->addr() <<" pkt arrived "<<gid<<" which I alredy sent or received from back\n";
		fflush(stdout);
		#endif
		return;
	}
	/* ----------MULTIPLE LEADER ELECTION CONTROL---------- */
	/* I've already received it from a nearest leader...reject this pkt! */
	if (rcv_pkt[gid].distance<dix) { 
		#ifdef DEBUG_BROADCAST
		   cout<<"BroadcastMsg - DROP - Node "<<((BroadcastbaseAgent*)agent_)->addr() <<" pkt arrived "<<gid<<" from farther leader  "<<dix<<" \n";
		fflush(stdout);
		#endif
		return;	
	}

	/* If past upper control I need to cancel the timer for this pkt and reschedule another one...another valid leader for this pkt has been elected and I have to enter into another broadcast phase! */
	timer->cancel();
	#ifdef DEBUG_BROADCAST
	    cout<<"BroadcastMsg - Node "<<((BroadcastbaseAgent*)agent_)->addr() <<" cancel timer for pkt "<<gid<<" because received from distance"<<dix<<" <= "<<rcv_pkt[gid].distance<<"\n";
	fflush(stdout);
	#endif
	int cw; //used for value of contention window
	double MaxRange;
	if (direction=='b') //setting of MaxRange
		MaxRange= fmr;	  
	else if(direction=='f')
		MaxRange= bmr;	
	//I get contention window (in slot)
	cw = compute_contwnd(distance,MaxRange);  
	Random::seed_heuristically();	 
	 //random slot from CW
	int rslot=Random::integer(cw);		
	double r=rslot* SLOT;
	
	#ifdef DEBUG_BROADCAST
	cout<<"BroadcastMsg - Node "<<((BroadcastbaseAgent*)agent_)->addr() <<" value choosen "<<rslot<<" inside CW which is "<<r<<" sec\n";
	fflush(stdout);
	#endif
	MyBCastCWTimer *cwt =new MyBCastCWTimer(this,gid,((BroadcastbaseAgent*)agent_)->addr(),direction,hops,slot_w);

	rcv_pkt[gid].cwt=cwt; 	//got new timer for new cw value 
	rcv_pkt[gid].distance=dix; //set distance from new leader
	rcv_pkt[gid].slot_choosen=rslot;
	cwt->resched(r);
	free(timer);
}

/* -- Compute Contention Window-- */
int BroadcastbaseApp::compute_contwnd(double distance,double MaxRange){
	int tmp;
	if (MaxRange==0) //avoid division by zero
		MaxRange=100; 

	if (MaxRange<distance) //used to get a positive number
		tmp=(int)((((distance-MaxRange)/MaxRange)*(CWMax-CWMin))+CWMin);
	else 
		tmp=(int)((((MaxRange-distance)/MaxRange)*(CWMax-CWMin))+CWMin);
	#ifdef DEBUG_BROADCAST
	  cout << "COMPUTED CW: Maxrange: "<<MaxRange<<" Distance: "<<distance<<" Result: "<<tmp<<"\n";
	fflush(stdout);
	#endif
	return(tmp);
}

/* --Function to find highest (maximum) value in array-- */
double BroadcastbaseApp::maxArray(double array[])
{
	int length = MAXWINDOW;
	double max = array[0]; // start with max = first element
	for(int i = 1; i<length; i++)
	{
		if(array[i] > max)
			max = array[i];
	}
	return max; // return highest value in array
}

/* --Parameter setting when msg is received from back-- */
void BroadcastbaseApp::set_parameters_back(BBCastData* data, double d,double mpx,double spx){
	
	double fmd_rcv;
	fmd_rcv=data->g_fmd();
	/* BMD updating */
	lbmd[ibmd]=d;
	ibmd=(ibmd+1)%MAXWINDOW;
	bmd=maxArray(lbmd);
	
	/* BMR updating */
	lbmr[ibmr]=fmd_rcv;
	ibmr=(ibmr+1)%MAXWINDOW;
	bmr=maxArray(lbmr);	
		
	//print: bmr time value addr pkt_id=flow_id
        #ifdef RANGENODE
	if (((BroadcastbaseAgent*)agent_)->addr() == RANGENODE) {
	#endif	
	fprintf (rangeFile,"bmr %.9f %f %d %ld\n",Scheduler::instance().clock(),bmr,((BroadcastbaseAgent*)agent_)->addr() ,data->g_id());
	fflush(rangeFile);
	#ifdef RANGENODE
	}
	#endif
		
}

/* --Parameter setting when msg is received from front-- */
void BroadcastbaseApp::set_parameters_front(BBCastData* data, double d,double mpx,double spx){	
	double bmd_rcv;
	bmd_rcv=data->g_bmd();

	/* FMD updating */
	lfmd[ifmd]=d;
	ifmd=(ifmd+1)%MAXWINDOW;
	fmd=maxArray(lfmd);

	/* FMR updating */
	lfmr[ifmr]=bmd_rcv;
	ifmr=(ifmr+1)%MAXWINDOW;
	fmr=maxArray(lfmr);
	
	//print: fmr time value addr pkt_id=flow_id
	#ifdef RANGENODE
	if (((BroadcastbaseAgent*)agent_)->addr() == RANGENODE) {
	#endif	
	fprintf (rangeFile,"fmr %.9f %f %d %ld\n",Scheduler::instance().clock(),fmr,((BroadcastbaseAgent*)agent_)->addr() ,data->g_id());
	fflush(rangeFile);
	#ifdef RANGENODE
	}
	#endif	
}

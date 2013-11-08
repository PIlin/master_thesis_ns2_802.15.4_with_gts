#ifndef ns_broadcast_base_h
#define ns_broadcast_base_h

//Broadcast agent class
class BroadcastbaseAgent : public Agent {

public:
	/* constructor */
	BroadcastbaseAgent(); 
	/* OTcl command interpreter */
	int command(int argc, const char*const* argv); 
	/* send packet */
	void sendbroadcastmsg( int realsize, BBCastData* data, long int id); 
	/* receive packet */
	virtual void recv(Packet*, Handler*); 
};

#endif 

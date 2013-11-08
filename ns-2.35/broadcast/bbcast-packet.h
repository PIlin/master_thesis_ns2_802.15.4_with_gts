#ifndef ns_bbcast_packet_h
#define ns_bbcast_packet_h

// Broadcast Packet Header
struct hdr_bbcast{
    double x_;	  //x sender coordinate
    double y_;    //y sender coordinate
    double fmd_;  //front max dist
    double bmd_;  //back max dist
    long int id_; //unique id for this packet (at application level)
    char dir_;    //direction for this packet
    int nhop_;    //number of hops till here
    long int slot_waiten_; 


     /* Warning: don't change!!! ns need to access at broadcast packet */
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static hdr_bbcast* access(const Packet* p) {
    	return (hdr_bbcast*) p->access(offset_);
    }
};

// User-level packet
class BBCastData : public AppData {
private:
	
	 double x_;
	 double  y_;
	 double fmd_;  //front max dist
	 double bmd_;  //back max dist
	 long int id_; //unique id for this packet (at application level)
	 char dir_;    //direction for this packet
	 int nhop_;    //number of hops till here
	 long int slot_waiten_; //slop waiten till this hop
    
public:
	BBCastData() : AppData(BBCAST_DATA) {}

	BBCastData(char * b);
 	/* setting fields */
	BBCastData(BBCastData& d) : AppData(d) 
	{ 
		
		x_=d.x_;
		y_=d.y_;
		fmd_=d.fmd_;
		bmd_=d.bmd_;
		id_=d.id_;
		dir_=d.dir_;
		nhop_=d.nhop_;
		slot_waiten_=d.slot_waiten_; 
		
	}

	/* getting fields */
	double  g_x()		{ return (x_); }
	double  g_y()		{ return (y_); }
	double g_fmd()		{ return (fmd_);}
	double g_bmd()		{ return (bmd_);}
	long int g_id()		{ return (id_);}
	char g_dir()		{ return (dir_);}
	char g_nhop()		{ return (nhop_);}
	long int g_slot_waiten(){ return (slot_waiten_);}
	//set id
	void set_id(long int id) {id_=id;}
	//set packet header
	void setHeader(hdr_bbcast *ih);
	
    
	virtual int size() const { return sizeof(BBCastData); }
	virtual AppData* copy() { return (new BBCastData(*this)); }
	void pack(char* buf) const;
	void print();
};
#endif

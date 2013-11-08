#include "ip.h"
#include "bbcast-packet.h"

int hdr_bbcast::offset_; 

// BBCast Header Class 
static class BBCastHeaderClass : public PacketHeaderClass {
public:
	BBCastHeaderClass() : PacketHeaderClass("PacketHeader/BBCast",
						    sizeof(hdr_bbcast)) {
		bind_offset(&hdr_bbcast::offset_);
	}
} class_hdr_bbcast;

//constructor
BBCastData::BBCastData(char* buf) : AppData(BBCAST_DATA)
{
	
	x_ = ((hdr_bbcast*)buf)->x_;
	y_ = ((hdr_bbcast*)buf)->y_;
	fmd_ = ((hdr_bbcast*)buf)->fmd_;
	bmd_ = ((hdr_bbcast*)buf)->bmd_;
	id_ = ((hdr_bbcast*)buf)->id_;
	dir_ = ((hdr_bbcast*)buf)->dir_;
	nhop_ = ((hdr_bbcast*)buf)->nhop_;
	slot_waiten_ = ((hdr_bbcast*)buf)->slot_waiten_;

}

//set fields for new packet
void BBCastData::pack(char* buf) const
{		
	((hdr_bbcast*)buf)->x_ = x_;
	((hdr_bbcast*)buf)->y_ = y_;
	((hdr_bbcast*)buf)->fmd_=fmd_;
	((hdr_bbcast*)buf)->bmd_=bmd_;
	((hdr_bbcast*)buf)->id_=id_;
	((hdr_bbcast*)buf)->dir_=dir_;
	((hdr_bbcast*)buf)->nhop_=nhop_;
	((hdr_bbcast*)buf)->slot_waiten_=slot_waiten_;
	
}
//set header fields
void BBCastData::setHeader(hdr_bbcast *ih)
{
	
	x_=ih->x_;
	y_=ih->y_;
	fmd_=ih->fmd_;
	bmd_=ih->bmd_;
	id_=ih->id_;
	dir_=ih->dir_; 
	nhop_=ih->nhop_;
	slot_waiten_=ih->slot_waiten_;
	
}
//print packet information
void BBCastData::print()
{	
	printf("X_	:\t%f\n",x_);
	printf("Y_	:\t%f\n",y_);
	printf("FMD_	:\t%f\n",fmd_);
	printf("BMD_	:\t%f\n",bmd_);
	printf("ID_	:\t%ld\n",id_);
	printf("Direction_	:\t%c\n",dir_);
	printf("HOPS_	:\t%d\n",nhop_);
	printf("SLOT WAITEN_	:\t%ld\n",slot_waiten_);
	fflush(stdout);
}

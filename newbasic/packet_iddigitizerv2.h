#ifndef __PACKET_IDDIGITIZERV2_H__
#define __PACKET_IDDIGITIZERV2_H__


#include "packet_w124.h"

#ifndef __CINT__
class WINDOWSEXPORT Packet_iddigitizerv2 : public  Packet_w4 {
#else
class  Packet_iddigitizerv2 : public  Packet_w4 {
#endif

public:
  Packet_iddigitizerv2( PACKET_ptr);
  ~Packet_iddigitizerv2();

  int    iValue(const int sample, const int ch);
  int    iValue(const int ,const char * what);
  long long   lValue(const int ,const char * what);
  void  dump ( OSTREAM& os = COUT) ;


protected:
  int decode ();



  int _evtnr;
  int _evtrollover;
  int _module_address;
  int _clock;
  int _clock_rollover;

  int _fem_slot[4];
  int _fem_evtnr[4];
  int _fem_clock[4];

  int _nsamples;
  int _nr_modules;

  int _even_checksum;
  int _odd_checksum;

  int _calculated_even_checksum;
  int _calculated_odd_checksum;

  int _even_checksum_ok;
  int _odd_checksum_ok;

  int _nchannels;
  int _is_decoded;

  int array[32][4*64];

};


#endif /* __PACKET_IDDIGITIZERV2_H__ */

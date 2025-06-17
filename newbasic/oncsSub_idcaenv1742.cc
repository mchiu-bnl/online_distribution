#include "oncsSub_idcaenv1742.h"
#include <cstring>


oncsSub_idcaenv1742::oncsSub_idcaenv1742(subevtdata_ptr data)
  :oncsSubevent_w4 (data)
{
  samples = 0;
  dlength = 0;
  EvtTimeTag = 0;
  LVDSPattern = 0;
  evnr = 0;
  freq=3;
  group_mask=0;
  int i;
  for ( i=0; i< 4; i++)
    {
      index_cell[i] = 0;
      tr_present[i] = 0;
      GroupTriggerTime[i] = 0;
    }

}
  
int *oncsSub_idcaenv1742::decode ( int *nwout)
{
  int *p;


  unsigned int *SubeventData = (unsigned int *) &SubeventHdr->data;


  // the first word must have  0xa in the MSB, and
  // has the total payload size in bits 0-27  
  if ( (( *SubeventData >> 28)  & 0xf ) != 0xa )
    {
      std::cout << "error in data structure" << std::endl;
      return 0;
    }
  dlength  = *SubeventData  & 0x0fffffff; 
 
  // word 1 has the group enable pattern
  group_mask = SubeventData[1] & 0xf;

  LVDSPattern = (SubeventData[1]>>8) & 0xffff;
  
  // word 2 has the event counter in bits 0-21
  evnr = SubeventData[2] & 0x3fffff;
  //  std::cout << "Evnt nr: " << evnr << std::endl;

  EvtTimeTag = (unsigned int) SubeventData[3];
	  
  // before we go through the groups indexed by group_index,
  // we take a peek into the first group to figure out how many samples we have.
  // the sample count is the same for all groups, although each group encodes this  
  // value again. We use this value here up front to allocate the right amount of memory
  // for the decoded waveforms. 

  int size = SubeventData[4] & 0xfff;
  samples = size / 3;

  p = new int [ samples * 8 * 4];
  memset(p, 0, samples * 8 * 4 * sizeof(int));

  // the trigger waveform, if any
  int *p_tr = new int [ samples  * 4];
  memset(p_tr, 0, samples * 4 * sizeof(int));
  decoded_data2 = p_tr;
  data2_length = samples *4;


  // we also extract the sampling frequency here (encoded in each group but 
  // the same for all)
  freq = (SubeventData[4] >> 16) & 3;

  
  //now we go through the groups
  int group_offset = 4;
  int group_nr;

  for ( group_nr=0; group_nr < 4; group_nr++)
    {
      int pos = 0;

      if  ( (group_mask >> group_nr) &1) // we have that group present
	{
	  unsigned int *groupdata = &(SubeventData[group_offset]); // first group
	  tr_present[group_nr] = (groupdata[0] >> 12) & 1;
	  index_cell[group_nr] = (groupdata[0] >> 20) & 0x3ff;


	  // std::cout << "group " << group_nr << " size:  " << size << std::endl;
	  // std::cout << "contains_tr: " << tr_present[group_nr] << std::endl;
	  // std::cout << "frequency  : " << freq << std::endl;
	  // std::cout << "index cell : " << index_cell[group_nr] << std::endl;

	  int s, ch;
	  pos = 1;

	  for ( s = 0; s < samples; s++ )
	    {
	      ch = 0;
	      p[group_nr*samples*8 + samples * ch++ + s] =( groupdata[pos] & 0xfff);
	      p[group_nr*samples*8 + samples * ch++ + s] =(groupdata[pos] >> 12) & 0xfff;
	      p[group_nr*samples*8 + samples * ch++ + s] = ((groupdata[pos] >> 24) & 0xff) + ( (groupdata[pos+1] & 0xf)<<8);
	      
	      p[group_nr*samples*8 + samples * ch++ + s] =(groupdata[pos+1] >> 4) & 0xfff;
	      p[group_nr*samples*8 + samples * ch++ + s] =(groupdata[pos+1] >> 16) & 0xfff;
	      p[group_nr*samples*8 + samples * ch++ + s] =((groupdata[pos+1] >> 28) & 0xf) + ( (groupdata[pos+2] & 0xff)<<4);
	      
	      p[group_nr*samples*8 + samples * ch++ + s] =(groupdata[pos+2] >> 8) & 0xfff;
	      p[group_nr*samples*8 + samples * ch++ + s] =(groupdata[pos+2] >> 20) & 0xfff;
	      pos +=3;

	    }
	  if ( tr_present[group_nr])
	    {
	      s = 0;
	      while (s < samples)
		{
		  p_tr[group_nr*samples + s++] = ( groupdata[pos] & 0xfff);
		  p_tr[group_nr*samples + s++] =(groupdata[pos] >> 12) & 0xfff;
		  p_tr[group_nr*samples + s++] = ((groupdata[pos] >> 24) & 0xff) + ( (groupdata[pos+1] & 0xf)<<8);
	      
		  p_tr[group_nr*samples + s++] =(groupdata[pos+1] >> 4) & 0xfff;
		  p_tr[group_nr*samples + s++] =(groupdata[pos+1] >> 16) & 0xfff;
		  p_tr[group_nr*samples + s++] =((groupdata[pos+1] >> 28) & 0xf) + ( (groupdata[pos+2] & 0xff)<<4);
	      
		  p_tr[group_nr*samples + s++] =(groupdata[pos+2] >> 8) & 0xfff;
		  p_tr[group_nr*samples + s++] =(groupdata[pos+2] >> 20) & 0xfff;
		  pos +=3;
		}
	    }
	  GroupTriggerTime[group_nr] = groupdata[pos] & 0x3fffffff;
	  group_offset += pos + 1;
	  
	}
    }

  // now see if we have an extended trigger time setting
  if ( GroupTriggerTime[0] != GroupTriggerTime[1])
    {
      GroupTriggerTime[0] |= ( GroupTriggerTime[1] << 30);
      GroupTriggerTime[1] = 0;
    }
  if ( GroupTriggerTime[2] != GroupTriggerTime[3])
    {
      GroupTriggerTime[2] |= ( GroupTriggerTime[3] << 30);
      GroupTriggerTime[3] = 0;
    }
  
  *nwout = samples*8 *4;

  return p;

}

int oncsSub_idcaenv1742::iValue(const int ch)
{

  if ( decoded_data1 == 0 ) decoded_data1 = decode(&data1_length);

  if ( ch < 0 || ch >= data1_length ) return 0;

  return decoded_data1[ch];

}

int oncsSub_idcaenv1742::iValue(const int sample, const int ch)
{

  if ( decoded_data1 == 0 ) decoded_data1 = decode(&data1_length);

  //if ( ch < 0 || ch >= 32 ) return 0;
  if ( ch < 0 || ch >= 36 ) return 0;
  if ( sample < 0 || sample >= samples ) return 0;

  //return decoded_data1[ch*samples + sample];
  return (ch < 32 ? decoded_data1[ch*samples + sample] : decoded_data2[(ch-32)*samples + sample]);

}

int oncsSub_idcaenv1742::iValue(const int n,const char *what)
{

  if ( decoded_data1 == 0 ) decoded_data1 = decode(&data1_length);

  if ( strcmp(what,"SAMPLES") == 0 )
  {
    return samples;
  }

  if ( strcmp(what,"EVNR") == 0 )
  {
    return evnr;
  }

  if ( strcmp(what,"TR0") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[n];
  }

  if ( strcmp(what,"TR1") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[2*samples + n];
  }

  if ( strcmp(what,"TR0-0") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[            n];
  }
  if ( strcmp(what,"TR0-1") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[1*samples + n];
  }
  if ( strcmp(what,"TR1-0") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[2*samples + n];
  }
  if ( strcmp(what,"TR1-1") == 0 )
  {
    if ( n <0 || n >=samples) return 0;

    return decoded_data2[3*samples + n];
  }

  if ( strcmp(what,"INDEXCELL") == 0 )
  {
    if ( n <0 || n >=4) return 0;

    return index_cell[n];
  }

  if ( strcmp(what,"TRPRESENT") == 0 )
  {
    if ( n <0 || n >=4) return 0;

    return tr_present[n];
  }

  if ( strcmp(what,"FREQUENCY") == 0 )
  {
    return freq;
  }

  if ( strcmp(what,"GROUPPRESENT") == 0 )
  {
    if ( n <0 || n >=4) return 0;
    return (group_mask >> n) &1;
  }

  if ( strcmp(what,"LVDSPATTERN") == 0 )
  {
    return LVDSPattern;
  }

  if ( strcmp(what,"EVENTTIME") == 0 )
  {
    return EvtTimeTag;
  }

  // if ( strcmp(what,"GROUPTRIGGERTIME") == 0 )
  // {
  //   if ( n <0 || n >=4) return 0;
  //   return GroupTriggerTime[n];
  // }


  return 0;

}

long long oncsSub_idcaenv1742::lValue(const int n, const char *what)
{
  if ( decoded_data1 == 0 ) decoded_data1 = decode(&data1_length);
  
  if ( strcmp(what,"GROUPTRIGGERTIME") == 0 )
  {
    if ( n <0 || n >=4) return 0;
    return GroupTriggerTime[n];
  }

  return 0;
}



void  oncsSub_idcaenv1742::dump ( OSTREAM& os )
{
  int i,j;
  //  int *SubeventData = &SubeventHdr->data;
  
  os << "Samples:  " << iValue(0,"SAMPLES") << std::endl;
  os << "Evt Nr:   " << iValue(0,"EVNR") << std::endl;
  os << "Evt Time: " << iValue(0,"EVENTTIME") << std::endl;
  //os << "LVDS:      " << iValue(0,"LVDSPATTERN") << std::endl;
  int f = iValue(0,"FREQUENCY") ;
  os << "Sample Frequency ";
  switch (f)
    {
    case 0:
      os << " 5 GS/s ";
      break;

    case 1:
      os << " 2.5 GS/s ";
      break;

    case 2:
      os << " 1 GS/s ";
      break;

    default:
      os << " Unknown ";
      break;
    }
  os << "("<< f << ")" << std::endl;

  os << "Group present:           ";
  for ( i = 0; i < 4; i++)
    {
      os<< iValue(i, "GROUPPRESENT") << "  ";
    }
  os << std::endl;


  os << "contains trigger sample: ";
  for ( i = 0; i < 4; i++)
    {
      os<< iValue(i, "TRPRESENT") << "  ";
    }
  os << std::endl;

  os << "index cell:              ";
  for ( i = 0; i < 4; i++)
    {
      os<<  iValue(i, "INDEXCELL") << "  ";
    }
  os << std::endl;

  os << "Group Trigger time       ";
  for ( i = 0; i < 4; i++)
    {
      os<<  std::setw(10) << lValue(i, "GROUPTRIGGERTIME") << "  ";
    }
  os << std::endl;

  
  for ( i = 0; i < samples ; i++)
    {
      os  << std::setw(4) << i << " |  ";

      for ( j = 0; j < 32 ; j++)
	{
	  
	  os << std::setw(4) << iValue(i,j) << " ";
	}
      os << " tr: " << std::setw(5) << iValue(i, "TR0") << " " << std::setw(5) << iValue(i, "TR1")<< std::endl;
    }
  
  os << std::endl;
}

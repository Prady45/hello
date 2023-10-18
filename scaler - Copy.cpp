#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <sstream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <map>
#include "scaler.h"
#include <sys/stat.h> 

#ifdef PLATFORM_NT
#include <direct.h> // for _mkdir function
#endif



using namespace std;

int scaler::log2(sint32 w)
{
   sint32 i ;
   sint32 ww ;

   i = -1 ;
   ww = w ;
   while(ww>0){
      i++ ;
      ww >>= 1 ;
   }
   if(w!=(int)pow(2.0,(double)i)) i++ ;
   return i ;
}

void scaler::dec2bin(sint64 num, int num_width, int *bin) 
{
   sint32 i ;
   uint64 temp;

   for(i=0;i<num_width;i++) bin[i] = 0 ;
   i = 0;
   if (num < 0) num += (sint64)pow(2.0,num_width);
   temp = (uint64)num;
   while(temp > 0){
      bin[i] = temp%2;      
      temp = temp/2 ;
      i ++ ;
   }
}

int96 scaler::int642int96(sint64 a)
{
   int96 result;
   sint64 temp;
   sint64 absa;

   temp = (sint64)pow(2.0,32);
   if (a >= 0) result.s = false;
   else        result.s = true;
   if (a < 0) absa = -a;
   else       absa = a;
   result.l = absa & 0xFFFFFFFF;
   absa = absa/temp;
   result.m = absa & 0xFFFFFFFF;
   result.h = 0;
   return result;
}

int96 scaler::int642int96(uint64 a)
{
   int96 result;
   uint64 temp;
   uint64 absa;

   temp = (sint64)pow(2.0,32);
   result.s = false;
   absa = a;
   result.l = absa & 0xFFFFFFFF;
   absa = absa/temp;
   result.m = absa & 0xFFFFFFFF;
   result.h = 0;
   return result;
}
sint64 scaler::int962int64(int96 a,int width)
{
   sint64 result;
   if(a.s) {
      if(a.h==0 && a.m==0 && a.l==0) {
         result = 1<<(width-1);
      } else {
         result = a.m;
         result = result<<32;
         result = result + a.l;
      }
      result = -result;
   } else {
      result = a.m;
      result = result<<32;
      result = result + a.l;
   }
   return result;
}

int96 scaler::int96pow2(unsigned int a, bool s)
{
   int96 result;

   result.s = s;
   if (a < 32) {
      result.h = 0; result.m = 0; result.l = 1<<a;
   } else if (a < 64) {
      result.h = 0; result.m = 1<<a; result.l = 0;
   } else {
      result.h = 1<<a; result.m = 0; result.l = 0;
   }
   return result;
}

int scaler::int96abscomp(int96 a, int96 b)
{
   int result;

   if (a.h > b.h)
      result = 1;
   else if (a.h < b.h)
      result = -1;
   else if (a.m > b.m)
      result = 1;
   else if (a.m < b.m)
      result = -1;
   else if (a.l > b.l)
      result = 1;
   else if (a.l < b.l)
      result = -1;
   else
      result = 0;
   return result;
}

int96 scaler::int96add(int96 a, int96 b)
{
   int96 result;

   int fh, fm, fl;

   if (a.h > b.h)       fh = 1;
   else if (a.h == b.h) fh = 0;
   else                 fh = -1;
   if (a.m > b.m)       fm = 1;
   else if (a.m == b.m) fm = 0;
   else                 fm = -1;
   if (a.l > b.l)       fl = 1;
   else if (a.l == b.l) fl = 0;
   else                 fl = -1;

   if (a.s == true && b.s == true) {
      result.s = true;
      result.l = a.l + b.l;
      result.m = a.m + b.m;
      result.h = a.h + b.h;
      if (result.l < a.l && result.l < b.l) result.m += 1;
      if (result.m < a.m && result.m < b.m) result.h += 1;
   } else if (a.s == false && b.s == false) {
      result.s = false;
      result.l = a.l + b.l;
      result.m = a.m + b.m;
      result.h = a.h + b.h;
      if (result.l < a.l && result.l < b.l) result.m += 1;
      if (result.m < a.m && result.m < b.m) result.h += 1;
   } else if (a.s == true && b.s == false) {
      if (fh == 1) {
         result.s = true;
         result.h = a.h - b.h;
         result.m = a.m - b.m;
         result.l = a.l - b.l;
         if (fl == -1) result.m -= 1;
         if ((result.m > a.m && result.m > b.m) || fm== -1) result.h -= 1;
      } else if (fh == -1) {
         result.s = false;
         result.h = b.h - a.h;
         result.m = b.m - a.m;
         result.l = b.l - a.l;
         if (fl == 1) result.m -= 1;
         if ((result.m > a.m && result.m > b.m) || fm == 1) result.h -= 1;
      } else if (fm == 1) {
         result.s = true;
         result.h = 0;
         result.m = a.m - b.m;
         result.l = a.l - b.l;
         if (fl == -1) result.m -= 1;
      } else if (fm == -1) {
         result.s = false;
         result.h = 0;
         result.m = b.m - a.m;
         result.l = b.l - a.l;
         if (fl == 1) result.m -= 1;
      } else if (fl == 1) {
         result.s = true;
         result.h = 0;
         result.m = 0;
         result.l = a.l - b.l;
      } else if (fl == -1) {
         result.s = false;
         result.h = 0;
         result.m = 0;
         result.l = b.l - a.l;
      } else {
         result.s = false;
         result.h = 0;
         result.m = 0;
         result.l = 0;
      }
   } else { // a false b true
      if (fh == 1) {
         result.s = false;
         result.h = a.h - b.h;
         result.m = a.m - b.m;
         result.l = a.l - b.l;
         if (fl == -1) result.m -= 1;
         if ((result.m > a.m && result.m > b.m) || fm == -1) result.h -= 1;
      } else if (fh == -1) {
         result.s = true;
         result.h = b.h - a.h;
         result.m = b.m - a.m;
         result.l = b.l - a.l;
         if (fl == 1) result.m -= 1;
         if ((result.m > a.m && result.m > b.m) || fm == 1) result.h -= 1;
      } else if (fm == 1) {
         result.s = false;
         result.h = 0;
         result.m = a.m - b.m;
         result.l = a.l - b.l;
         if (fl == -1) result.m -= 1;
      } else if (fm == -1) {
         result.s = true;
         result.h = 0;
         result.m = b.m - a.m;
         result.l = b.l - a.l;
         if (fl == 1) result.m -= 1;
      } else if (fl == 1) {
         result.s = false;
         result.h = 0;
         result.m = 0;
         result.l = a.l - b.l;
      } else if (fl == -1) {
         result.s = true;
         result.h = 0;
         result.m = 0;
         result.l = b.l - a.l;
      } else {
         result.s = false;
         result.h = 0;
         result.m = 0;
         result.l = 0;
      }
   }
   return result;
}

bool scaler::int962hex(int96 a, int *b, int width)
{
   int96 temp;
   uint32 aa;
   int hexnum = (width+3)/4;
   int ii;

   temp.h = a.h;
   temp.m = a.m;
   temp.l = a.l;
   temp.s = a.s;
   for (ii=0;ii<hexnum;ii++) b[ii] = 0;

   if (temp.s == true) {
      temp.l = ~temp.l;
      temp.m = ~temp.m;
      temp.h = ~temp.h;
      aa = temp.l;
      temp.l += 1;
      if (temp.l < aa) {
         aa = temp.m;
         temp.m += 1;
         if (temp.m < aa) temp.h += 1;
      }
   }
   if (width <= 32) {
      for (ii=0;ii<hexnum;ii++) {
         b[ii] = temp.l%16;
         temp.l = temp.l >> 4;
         if (ii == hexnum - 1) {
            if (width%4 == 1)
               b[ii] = b[ii] & 0x00000001;
            else if (width%4 == 2)
               b[ii] = b[ii] & 0x00000003;
            else if (width%4 == 3)
               b[ii] = b[ii] & 0x00000007;
            else
               b[ii] = b[ii] & 0x0000000f;
         }
      }
   } else if (width <= 64) {
      for (ii=0;ii<8;ii++) {
         b[ii] = temp.l%16;
         temp.l = temp.l >> 4;
      }
      for (ii=8;ii<hexnum;ii++) {
         b[ii] = temp.m%16;
         temp.m = temp.m >> 4;
         if (ii == hexnum - 1) {
            if (width%4 == 1)
               b[ii] = b[ii] & 0x00000001;
            else if (width%4 == 2)
               b[ii] = b[ii] & 0x00000003;
            else if (width%4 == 3)
               b[ii] = b[ii] & 0x00000007;
            else
               b[ii] = b[ii] & 0x0000000f;
         }
      }
   } else {
      for (ii=0;ii<8;ii++) {
         b[ii] = temp.l%16;
         temp.l = temp.l >> 4;
      }
      for (ii=8;ii<16;ii++) {
         b[ii] = temp.m%16;
         temp.m = temp.m >> 4;
      }
      for (ii=16;ii<hexnum;ii++) {
         b[ii] = temp.h%16;
         temp.h = temp.h >> 4;
         if (ii == hexnum - 1) {
            if (width%4 == 1)
               b[ii] = b[ii] & 0x00000001;
            else if (width%4 == 2)
               b[ii] = b[ii] & 0x00000003;
            else if (width%4 == 3)
               b[ii] = b[ii] & 0x00000007;
            else
               b[ii] = b[ii] & 0x0000000f;
         }
      }
   }
   if (a.s == true && a.h == 0 && a.m == 0 && a.l == 0) {
      for (ii=0;ii<(width+3)/4-1;ii++) b[ii] = 0;
      if (width%4 == 0)
         b[(width+3)/4-1] = 8;
      else if (width%4 == 1)
         b[(width+3)/4-1] = 1;
      else if (width%4 == 2)
         b[(width+3)/4-1] = 2;
      else if (width%4 == 3)
         b[(width+3)/4-1] = 4;
   }
   return false;
}

int scaler::get_argu(int argc, char *argv[])
{
   FILE *fh;
   if (argc < 1) {
      printf("At least the argument of LPC should be added\n");
      return true;
   }
   GENGOLDEN    = false;
   DEBUG        = false;
   LOADSTIMULUS = false;
   LOADBMPFILE  = false;
   LOADYUVFILE  = false;
   
   strcpy(LDC_NAME, argv[1]);
  
   if (argc > 2) {
      if (strcmp(argv[2],"-d") == 0) {
         DEBUG = true;
         GENGOLDEN = true;
         LOADSTIMULUS = false;
         if(argc > 3) {
            if(atoi(argv[3]) > 3) {
               NUM_FRAME = atoi(argv[3]);
            }
         }
      } else if(strcmp(argv[2],"-g") == 0) {
         DEBUG = false;
         GENGOLDEN = true;
         LOADSTIMULUS = false;
         if(argc > 3) {
            if(atoi(argv[3]) > 3) {
               NUM_FRAME = atoi(argv[3]);
            }
         }
      } else if(strcmp(argv[2],"-l") == 0) {
         DEBUG = false;
         GENGOLDEN = true;
         LOADSTIMULUS = true;
         if(argc > 3)
            strcpy(STIMULUSFILE, argv[3]);
         else {
            printf("The stimulus file should be added\n");
            return -9;
         }
     } else if(strcmp(argv[2],"-b") == 0) {
         DEBUG = false;
         GENGOLDEN = true;
         LOADSTIMULUS = true;
         LOADBMPFILE = true;
         if(argc > 3)
            strcpy(STIMULUSFILE, argv[3]);
         else {
            printf("The stimulus file should be added\n");
            return -9;
         }
     } else if(strcmp(argv[2],"-y") == 0) {
         DEBUG = false;
         GENGOLDEN = true;
         LOADSTIMULUS = true;
         LOADYUVFILE = true;
         if(argc > 3)
            strcpy(STIMULUSFILE, argv[3]);
         else {
            printf("The stimulus file should be added\n");
            return -9;
         }
     }
   }


   if ((fh = fopen(LDC_NAME, "r")) == NULL) {
       printf("LDC file %s does not exist\n", LDC_NAME);
       return true;
   }
   else {
       fclose(fh);
   }

   return false;
}

bool scaler::parselpc()
{
   FILE *fh = NULL;
   char fline[1000];
   char *str_value;
   int  GUI_SYMMETRY;

   GUI_SYMMETRY = 0;
   fh = fopen(LPC_NAME, "r");
   if (fh == NULL) {
       printf("Error opening file.\n");
       return true;
   }
   while(fgets(fline,1000,fh) != NULL) {
      if(strstr(fline,"//") == NULL) {
         // parse FAMILY family
         if (strstr(fline,"Family=") != NULL) {
            str_value = strstr(fline,"Family") + strlen("Family")+1;
            strcpy(FAMILY,str_value);
            if      (strstr(FAMILY,"ep5g00p")  || strstr(FAMILY,"latticeecp")   ) strcpy(DEVICE,"ECP");
            else if (strstr(FAMILY,"ep5g00")   || strstr(FAMILY,"latticeec")    ) strcpy(DEVICE,"EC");
            else if (strstr(FAMILY,"ep5a00s")  || strstr(FAMILY,"latticeecp2s") ) strcpy(DEVICE,"ECP2");
            else if (strstr(FAMILY,"ep5m00s")  || strstr(FAMILY,"latticeecp2ms")) strcpy(DEVICE,"ECP2M");
            else if (strstr(FAMILY,"ep5a00")   || strstr(FAMILY,"latticeecp2")  ) strcpy(DEVICE,"ECP2");
            else if (strstr(FAMILY,"ep5m00")   || strstr(FAMILY,"latticeecp2m") ) strcpy(DEVICE,"ECP2M");
            else if (strstr(FAMILY,"ep5c00")   || strstr(FAMILY,"latticeecp3")  ) strcpy(DEVICE,"ECP3");
            else if (strstr(FAMILY,"ep5d00")   || strstr(FAMILY,       "ecp5")  ) strcpy(DEVICE,"ECP5U");
            else if (strstr(FAMILY,"sa5p00")   || strstr(FAMILY,       "ecp5u") ) strcpy(DEVICE,"ECP5U");
            else if (strstr(FAMILY,"sa5p00a")  || strstr(FAMILY,       "ecp5ua")) strcpy(DEVICE,"ECP5U");
            else if (strstr(FAMILY,"sa5p00m")  || strstr(FAMILY,       "ecp5um")) strcpy(DEVICE,"ECP5U");
            else if (strstr(FAMILY,"or5scm00") || strstr(FAMILY,"latticescm")   ) strcpy(DEVICE,"SCM");
            else if (strstr(FAMILY,"or5s00")   || strstr(FAMILY,"latticesc")    ) strcpy(DEVICE,"SC");
            else if (strstr(FAMILY,"mg5g00")   || strstr(FAMILY,"latticexp")    ) strcpy(DEVICE,"XP");
            else if (strstr(FAMILY,"mg5a00")   || strstr(FAMILY,"latticexp2")   ) strcpy(DEVICE,"XP2");
            else                                                                  strcpy(DEVICE,"ECP2");
         }
         // parse Part Type
         if (strstr(fline,"PartType=") != NULL) {
            str_value = strstr(fline,"PartType") + strlen("PartType")+1;
            strcpy(PART_TYPE,str_value);
         }
         // SCALER_NAME
         if (strstr(fline,"ModuleName") != NULL) {
            str_value = strstr(fline,"ModuleName") + strlen("ModuleName")+1;
            strcpy(SCALER_NAME,str_value);
            #ifdef PLATFORM_NT
               if (strstr(SCALER_NAME,"\n") != NULL) strtok(SCALER_NAME,"\n");
               if (strstr(SCALER_NAME,"\n") == SCALER_NAME) strcpy(SCALER_NAME,"");
            #else
               if (strstr(SCALER_NAME,"\r") != NULL) strtok(SCALER_NAME,"\r");
               else if (strstr(SCALER_NAME,"\n") != NULL) strtok(SCALER_NAME,"\n");
            #endif
         }
         // numplane
         if (strstr(fline,"numplane") != NULL) {
            str_value = strstr(fline,"numplane") + strlen("numplane")+1;
            NUM_PLANE = atoi(str_value);
         }
         // video input width
         if (strstr(fline,"vinwidth") != NULL) {
            str_value = strstr(fline,"vinwidth") + strlen("vinwidth")+1;
            VINWIDTH = atoi(str_value);
         }
         // video input height
         if (strstr(fline,"vinheight") != NULL) {
            str_value = strstr(fline,"vinheight") + strlen("vinheight")+1;
            VINHEIGHT = atoi(str_value);
         }
         // video output width
         if (strstr(fline,"voutwidth") != NULL) {
            str_value = strstr(fline,"voutwidth") + strlen("voutwidth")+1;
            VOUTWIDTH = atoi(str_value);
         }
         // video output height
         if (strstr(fline,"voutheight") != NULL) {
            str_value = strstr(fline,"voutheight") + strlen("voutheight")+1;
            VOUTHEIGHT = atoi(str_value);
         }
         // parallel
         if (strstr(fline,"parallel") != NULL) {
            str_value = strstr(fline,"parallel") + strlen("parallel")+1;
            PARALLEL = atoi(str_value);
         }
         // ycbcr422
         if (strstr(fline,"ycbcr422") != NULL) {
            str_value = strstr(fline,"ycbcr422") + strlen("ycbcr422")+1;
            YCBCR422 = atoi(str_value);
         }
         // ycbcr444
         if (strstr(fline,"ycbcr444") != NULL) {
            str_value = strstr(fline,"ycbcr444") + strlen("ycbcr444")+1;
            YCBCR444 = atoi(str_value);
         }
         // algorithm kernel
         if (strstr(fline,"kernel") != NULL) {
            str_value = strstr(fline,"kernel") + strlen("kernel")+1;
            KERNEL = atoi(str_value);
         }   // delete memory
         // vftaps
         if (strstr(fline,"vftaps") != NULL) {
            str_value = strstr(fline,"vftaps") + strlen("vftaps")+1;
            VFTAPS = atoi(str_value);
         }
         // hftaps
         if (strstr(fline,"hftaps") != NULL) {
            str_value = strstr(fline,"hftaps") + strlen("hftaps")+1;
            HFTAPS = atoi(str_value);
         }
         // vfphases
         if (strstr(fline,"vfphases") != NULL) {
            str_value = strstr(fline,"vfphases") + strlen("vfphases")+1;
            VFPHASES = atoi(str_value);
         }
         // hfphases
         if (strstr(fline,"hfphases") != NULL) {
            str_value = strstr(fline,"hfphases") + strlen("hfphases")+1;
            HFPHASES = atoi(str_value);
         }
         // dynamic scaling
         if (strstr(fline,"dynamic") != NULL) {
            str_value = strstr(fline,"dynamic") + strlen("dynamic")+1;
            DYNAMIC = atoi(str_value);
         }
         // edge apative
         if (strstr(fline,"adaptive") != NULL) {
            str_value = strstr(fline,"adaptive") + strlen("adaptive")+1;
            ADAPTIVE = atoi(str_value);
         }
         // separate pclk
         if (strstr(fline,"seppclk") != NULL) {
            str_value = strstr(fline,"seppclk") + strlen("seppclk")+1;
            SEPPCLK = atoi(str_value);
         }
         // coeff_type
         if (strstr(fline,"ctype") != NULL) {
            str_value = strstr(fline,"ctype") + strlen("ctype")+1;
            if (strstr(str_value,"Signed") != NULL)   COEFF_TYPE = 1;
            else                                      COEFF_TYPE = 0;
         }
         // chroma resample
         if (strstr(fline,"resample") != NULL) {
            str_value = strstr(fline,"resample") + strlen("resample")+1;
            if (strstr(str_value,"Auto") != NULL)          RESAMPLE = 2;
            else if (strstr(str_value,"Average") != NULL)  RESAMPLE = 1;
            else                                           RESAMPLE = 0;
         }
         // dinwidth
         if (strstr(fline,"dinwidth") != NULL) {
            str_value = strstr(fline,"dinwidth") + strlen("dinwidth")+1;
            DIN_WIDTH = atoi(str_value);
         }
         // cwidth
         if (strstr(fline,"cwidth") != NULL) {
            str_value = strstr(fline,"cwidth") + strlen("cwidth")+1;
            COEFF_WIDTH = atoi(str_value);
         }
         //// moutwidth
         //if (strstr(fline,"moutwidth") != NULL) {
         //   str_value = strstr(fline,"moutwidth") + strlen("moutwidth")+1;
         //   MOUT_WIDTH = atoi(str_value);
         //}
         // doutwidth
         if (strstr(fline,"doutwidth") != NULL) {
            str_value = strstr(fline,"doutwidth") + strlen("doutwidth")+1;
            DOUT_WIDTH = atoi(str_value);
         }
         //// doutpoints
         //if (strstr(fline,"doutpoints") != NULL) {
         //   str_value = strstr(fline,"doutpoints") + strlen("doutpoints")+1;
         //   DOUT_POINTS = atoi(str_value);
         //}
         // pbuswidth
         if (strstr(fline,"pbuswidth") != NULL) {
            str_value = strstr(fline,"pbuswidth") + strlen("pbuswidth")+1;
            PBUSWIDTH = atoi(str_value);
         }
         // lbuffer
         if (strstr(fline,"lbuffer") != NULL) {
            str_value = strstr(fline,"lbuffer") + strlen("lbuffer")+1;
            if (strstr(str_value,"EBR") != NULL)               LBUFFER = 1;
            else if (strstr(str_value,"Distributed") != NULL)  LBUFFER = 0;
            else                                               LBUFFER = 0;
         }
         // vcbuffer
         if (strstr(fline,"vcbuffer") != NULL) {
            str_value = strstr(fline,"vcbuffer") + strlen("vcbuffer")+1;
            if (strstr(str_value,"EBR") != NULL)               VCBUFFER = 1;
            else if (strstr(str_value,"Distributed") != NULL)  VCBUFFER = 0;
            else                                               VCBUFFER = 0;
         }
         // hcbuffer
         if (strstr(fline,"hcbuffer") != NULL) {
            str_value = strstr(fline,"hcbuffer") + strlen("hcbuffer")+1;
            if (strstr(str_value,"EBR") != NULL)               HCBUFFER = 1;
            else if (strstr(str_value,"Distributed") != NULL)  HCBUFFER = 0;
            else                                               HCBUFFER = 0;
         }
         // sharecbuffer
         if (strstr(fline,"sharecbuffer") != NULL) {
            str_value = strstr(fline,"sharecbuffer") + strlen("sharecbuffer")+1;
            SHARE_CMEM = atoi(str_value);
         }
         // multtype
         if (strstr(fline,"multtype") != NULL) {
            str_value = strstr(fline,"multtype") + strlen("multtype")+1;
            if (strstr(str_value,"DSP") != NULL)               MULTTYPE = 1;
            else                                               MULTTYPE = 0;
         }
         // highspeed
         if (strstr(fline,"highspeed") != NULL) {
            str_value = strstr(fline,"highspeed") + strlen("highspeed")+1;
            HIGHSPEED = atoi(str_value);
         }
         // lsbmethod
         if (strstr(fline,"lsbmethod") != NULL) {
            str_value = strstr(fline,"lsbmethod") + strlen("lsbmethod")+1;
            if (strstr(str_value,"Truncation") != NULL)   LSB_METHOD = 0;
            else if (strstr(str_value,"Normal") != NULL)  LSB_METHOD = 2;
            else                                          LSB_METHOD = 4;
         }
         // number of cbanks
         if (strstr(fline,"cbanks") != NULL) {
            str_value = strstr(fline,"cbanks") + strlen("cbanks")+1;
            CBANKS = atoi(str_value);
         }
         // coeffile, from 0 to 3
         if (strstr(fline,"coefile_0") != NULL) {
            str_value = strstr(fline,"coefile_0") + strlen("coefile_0")+1;
            strcpy(COEFILES[0],str_value);
            if (strstr(COEFILES[0],"\n") != NULL) strtok(COEFILES[0],"\n");
         }
         if (strstr(fline,"coefile_1") != NULL) {
            str_value = strstr(fline,"coefile_1") + strlen("coefile_1")+1;
            strcpy(COEFILES[1],str_value);
            if (strstr(COEFILES[1],"\n") != NULL) strtok(COEFILES[1],"\n");
         }
         if (strstr(fline,"coefile_2") != NULL) {
            str_value = strstr(fline,"coefile_2") + strlen("coefile_2")+1;
            strcpy(COEFILES[2],str_value);
            if (strstr(COEFILES[2],"\n") != NULL) strtok(COEFILES[2],"\n");
         }
         if (strstr(fline,"coefile_3") != NULL) {
            str_value = strstr(fline,"coefile_3") + strlen("coefile_3")+1;
            strcpy(COEFILES[3],str_value);
            if (strstr(COEFILES[3],"\n") != NULL) strtok(COEFILES[3],"\n");
         }
         // frmports
         if (strstr(fline,"frmports") != NULL) {
            str_value = strstr(fline,"frmports") + strlen("frmports")+1;
            FRMPORTS = atoi(str_value);
         }
         // resync
         if (strstr(fline,"resync") != NULL) {
            str_value = strstr(fline,"resync") + strlen("resync")+1;
            RESYNC = atoi(str_value);
         }
         // sr
         if (strstr(fline,"sr") != NULL) {
            str_value = strstr(fline,"sr") + strlen("sr")+1;
            SR = atoi(str_value);
         }
         // ce
         if (strstr(fline,"ce") != NULL) {
            str_value = strstr(fline,"ce") + strlen("ce")+1;
            CE = atoi(str_value);
         }
         //// tagswidth
         //if (strstr(fline,"tagswidth") != NULL) {
         //   str_value = strstr(fline,"tagswidth") + strlen("tagswidth")+1;
         //   TAGS_WIDTH= atoi(str_value);
         //}
         //----only for verification
         // frmwidth_0
         if (strstr(fline,"frmw_0") != NULL) {
            str_value = strstr(fline,"frmw_0") + strlen("frmw_0")+1;
            CUR_FRMWIDTH[0] = atoi(str_value);
         }
         // frmheight_0
         if (strstr(fline,"frmh_0") != NULL) {
            str_value = strstr(fline,"frmh_0") + strlen("frmh_0")+1;
            CUR_FRMHEIGHT[0] = atoi(str_value);
         }
         // frmwidth_1
         if (strstr(fline,"frmw_1") != NULL) {
            str_value = strstr(fline,"frmw_1") + strlen("frmw_1")+1;
            CUR_FRMWIDTH[1] = atoi(str_value);
         }
         // frmheight_1
         if (strstr(fline,"frmh_1") != NULL) {
            str_value = strstr(fline,"frmh_1") + strlen("frmh_1")+1;
            CUR_FRMHEIGHT[1] = atoi(str_value);
         }
         // frmwidth_2
         if (strstr(fline,"frmw_2") != NULL) {
            str_value = strstr(fline,"frmw_2") + strlen("frmw_2")+1;
            CUR_FRMWIDTH[2] = atoi(str_value);
         }
         // frmheight_2
         if (strstr(fline,"frmh_2") != NULL) {
            str_value = strstr(fline,"frmh_2") + strlen("frmh_2")+1;
            CUR_FRMHEIGHT[2] = atoi(str_value);
         }
         // outwidth_0
         if (strstr(fline,"outw_0") != NULL) {
            str_value = strstr(fline,"outw_0") + strlen("outw_0")+1;
            CUR_OUTWIDTH[0] = atoi(str_value);
         }
         // outheight_0
         if (strstr(fline,"outh_0") != NULL) {
            str_value = strstr(fline,"outh_0") + strlen("outh_0")+1;
            CUR_OUTHEIGHT[0] = atoi(str_value);
         }
         // outwidth_1
         if (strstr(fline,"outw_1") != NULL) {
            str_value = strstr(fline,"outw_1") + strlen("outw_1")+1;
            CUR_OUTWIDTH[1] = atoi(str_value);
         }
         // outheight_1
         if (strstr(fline,"outh_1") != NULL) {
            str_value = strstr(fline,"outh_1") + strlen("outh_1")+1;
            CUR_OUTHEIGHT[1] = atoi(str_value);
         }
         // outwidth_2
         if (strstr(fline,"outw_2") != NULL) {
            str_value = strstr(fline,"outw_2") + strlen("outw_2")+1;
            CUR_OUTWIDTH[2] = atoi(str_value);
         }
         // outheight_2
         if (strstr(fline,"outh_2") != NULL) {
            str_value = strstr(fline,"outh_2") + strlen("outh_2")+1;
            CUR_OUTHEIGHT[2] = atoi(str_value);
         }
         // EDGE THRESHOLD
         if (strstr(fline,"edge_th") != NULL) {
            str_value = strstr(fline,"edge_th") + strlen("edge_th")+1;
            EDGE_TH = atoi(str_value);
         }
         // FSCALE
         if (strstr(fline,"fscale") != NULL) {
            str_value = strstr(fline,"fscale") + strlen("fscale")+1;
            FSCALE = atof(str_value);
         }
         // CSETS_CS
         if (strstr(fline,"csets") != NULL) {
            str_value = strstr(fline,"csets") + strlen("csets")+1;
            CSETS_CS = atoi(str_value);
         }
         // SEED
         if (strstr(fline,"seed") != NULL) {
            str_value = strstr(fline,"seed") + strlen("seed")+1;
            SEED = atoi(str_value);
         }
      }
   }
   fclose(fh);

   if(KERNEL==4) {
      if(COEFF_TYPE) COEFF_POINTS = COEFF_WIDTH-1;
      else           COEFF_POINTS = COEFF_WIDTH;
      MOUT_WIDTH     = DIN_WIDTH;// force vout to positive value
   }

   if (DIN_TYPE) {
      max_din = (sint64)pow(2.0,DIN_WIDTH-1)-1;
      min_din = -(sint64)pow(2.0,DIN_WIDTH-1);
   } else {
      max_din = (sint64)pow(2.0,DIN_WIDTH)-1;
      min_din = 0;
   }
   if (COEFF_TYPE) {
      max_coe = (sint64)pow(2.0,COEFF_WIDTH-1)-1;
      min_coe = -(sint64)pow(2.0,COEFF_WIDTH-1);
   } else {
      max_coe = (sint64)pow(2.0,COEFF_WIDTH)-1;
      min_coe = 0;
   }
   if(LSB_METHOD==1 || LSB_METHOD==3) {// not support
      LSB_METHOD = 0;
   }
   if(KERNEL<4)         CBANKS = 1;
   else if(ADAPTIVE==1) CBANKS = 2;

   //DOUT_WIDTH   = DIN_WIDTH;
   TOP_PAD      = (VFTAPS-1)/2;
   LEFT_PAD     = (HFTAPS-1)/2;
   BOTTOM_PAD   = VFTAPS-1-TOP_PAD;
   RIGHT_PAD    = HFTAPS-1-LEFT_PAD;
   IMAGE_HEIGHT = VINHEIGHT+TOP_PAD+BOTTOM_PAD;
   IMAGE_WIDTH  = VINWIDTH +LEFT_PAD+RIGHT_PAD;
   FOUT_HEIGHT  = VOUTHEIGHT;
   FOUT_WIDTH   = VOUTWIDTH;
   DOUT_POINTS  = DOUT_WIDTH-DIN_WIDTH;//fix the bug of 8-bit to 10-bit
   srand(SEED);

   //actual number of window coeffs
   if(VFPHASES>VOUTHEIGHT) VFCBPWIDTH  = log2(VFPHASES);
   else                    VFCBPWIDTH  = log2(VOUTHEIGHT);
   if(HFPHASES>VOUTWIDTH)  HFCBPWIDTH  = log2(HFPHASES);
   else                    HFCBPWIDTH  = log2(VOUTWIDTH);

   VFBVALUE    = (sint64)1<<VFCBPWIDTH;
   HFBVALUE    = (sint64)1<<HFCBPWIDTH;
   VDFACTOR    = floor(((double)VFBVALUE*VINHEIGHT)/VOUTHEIGHT);
   HDFACTOR    = floor(((double)HFBVALUE*VINWIDTH)/VOUTWIDTH);
   VFCWIDTH    = log2(VINHEIGHT) + VFCBPWIDTH;
   HFCWIDTH    = log2(VINWIDTH) + HFCBPWIDTH;
   YFULL_WIDTH =  DIN_WIDTH+COEFF_WIDTH+log2(VFTAPS);
   XFULL_WIDTH = MOUT_WIDTH+COEFF_WIDTH+log2(HFTAPS);

   if(DYNAMIC==0) {
      CUR_FRMWIDTH[0]  = VINWIDTH-1;
      CUR_FRMWIDTH[1]  = CUR_FRMWIDTH[0];
      CUR_FRMWIDTH[2]  = CUR_FRMWIDTH[0];
      CUR_FRMHEIGHT[0] = VINHEIGHT-1;
      CUR_FRMHEIGHT[1] = CUR_FRMHEIGHT[0];
      CUR_FRMHEIGHT[2] = CUR_FRMHEIGHT[0];
   } else {
      if(CUR_FRMWIDTH[0]  < 0) CUR_FRMWIDTH[0]  = VINWIDTH-1;
      if(CUR_FRMWIDTH[1]  < 0) CUR_FRMWIDTH[1]  = CUR_FRMWIDTH[0] - 10;
      if(CUR_FRMWIDTH[2]  < 0) CUR_FRMWIDTH[2]  = CUR_FRMWIDTH[0] - 20;
      if(CUR_FRMHEIGHT[0] < 0) CUR_FRMHEIGHT[0] = VINHEIGHT-1;
      if(CUR_FRMHEIGHT[1] < 0) CUR_FRMHEIGHT[1] = CUR_FRMHEIGHT[0] - 10;
      if(CUR_FRMHEIGHT[2] < 0) CUR_FRMHEIGHT[2] = CUR_FRMHEIGHT[0] - 20;
   }
   if(DYNAMIC==0) {
      CUR_OUTWIDTH[0]  = VOUTWIDTH-1;
      CUR_OUTWIDTH[1]  = CUR_OUTWIDTH[0];
      CUR_OUTWIDTH[2]  = CUR_OUTWIDTH[0];
      CUR_OUTHEIGHT[0] = VOUTHEIGHT-1;
      CUR_OUTHEIGHT[1] = CUR_OUTHEIGHT[0];
      CUR_OUTHEIGHT[2] = CUR_OUTHEIGHT[0];
   } else {
      if(CUR_OUTWIDTH[0]  < 0) CUR_OUTWIDTH[0]  = VOUTWIDTH-1;
      if(CUR_OUTWIDTH[1]  < 0) CUR_OUTWIDTH[1]  = ((CUR_OUTWIDTH[0]+1)/4)*2-1;
      if(CUR_OUTWIDTH[2]  < 0) CUR_OUTWIDTH[2]  = ((CUR_OUTWIDTH[0]+1)/8)*2-1;
      if(CUR_OUTHEIGHT[0] < 0) CUR_OUTHEIGHT[0] = VOUTHEIGHT-1;
      if(CUR_OUTHEIGHT[1] < 0) CUR_OUTHEIGHT[1] = (CUR_OUTHEIGHT[0]+1)/2-1;
      if(CUR_OUTHEIGHT[2] < 0) CUR_OUTHEIGHT[2] = (CUR_OUTHEIGHT[0]+1)/4-1;
   }
   {// must larger than 32
      if(CUR_FRMWIDTH[0]  < 31) CUR_FRMWIDTH[0]  = 31;
      if(CUR_FRMWIDTH[1]  < 31) CUR_FRMWIDTH[1]  = 31;
      if(CUR_FRMWIDTH[2]  < 31) CUR_FRMWIDTH[2]  = 31;
      if(CUR_FRMHEIGHT[0] < 31) CUR_FRMHEIGHT[0] = 31;
      if(CUR_FRMHEIGHT[1] < 31) CUR_FRMHEIGHT[1] = 31;
      if(CUR_FRMHEIGHT[2] < 31) CUR_FRMHEIGHT[2] = 31;
      if(CUR_OUTWIDTH[0]  < 31) CUR_OUTWIDTH[0]  = 31;
      if(CUR_OUTWIDTH[1]  < 31) CUR_OUTWIDTH[1]  = 31;
      if(CUR_OUTWIDTH[2]  < 31) CUR_OUTWIDTH[2]  = 31;
      if(CUR_OUTHEIGHT[0] < 31) CUR_OUTHEIGHT[0] = 31;
      if(CUR_OUTHEIGHT[1] < 31) CUR_OUTHEIGHT[1] = 31;
      if(CUR_OUTHEIGHT[2] < 31) CUR_OUTHEIGHT[2] = 31;
   }
   return false;
}


bool scaler::parseldc()
{
    FILE* fh = NULL;
    char fline[1000];
    char str_value[100];
    int  GUI_SYMMETRY;

    GUI_SYMMETRY = 0;

    std::ifstream file(LDC_NAME);  // open the file for input
    std::string line;
    std::map<std::string, std::string> settings;  // use a map to store the settings
    
    while (std::getline(file, line)) {
        size_t pos = line.find("set ");
        if (pos != std::string::npos) {  // if the line contains "set "
            line.erase(pos, 4);  // erase "set " from the line
            pos = line.find(" ");
            std::string key = line.substr(0, pos);  // extract the key
            line.erase(0, pos + 1);  // erase the key and the space after it
            pos = line.find("\"");
            line.erase(0, pos + 1);  // erase the opening quote
            pos = line.find("\"");
            std::string value = line.substr(0, pos);  // extract the value
            settings[key] = value;  // store the setting in the map
        }
    }
        

    //for (const auto& setting : settings) {
    //    std::cout << setting.first << " = " << setting.second << std::endl;
    //}

    if (settings.find("device") != settings.end())
    {
        strcpy(DEVICE, (settings.at("device")).c_str());
    }

    if (settings.find("SCALER_NAME") != settings.end())
    {
        strcpy(str_value, (settings.at("SCALER_NAME")).c_str());
        strcpy(SCALER_NAME, str_value);

#ifdef PLATFORM_NT
        if (strstr(SCALER_NAME, "\n") != NULL) strtok(SCALER_NAME, "\n");
        if (strstr(SCALER_NAME, "\n") == SCALER_NAME) strcpy(SCALER_NAME, "");
#else
        if (strstr(SCALER_NAME, "\r") != NULL) strtok(SCALER_NAME, "\r");
        else if (strstr(SCALER_NAME, "\n") != NULL) strtok(SCALER_NAME, "\n");
#endif

    }

    if (settings.find("NUM_PLANE") != settings.end()) 
    {
        NUM_PLANE = atoi((settings.at("NUM_PLANE")).c_str());
    }
    if (settings.find("VINWIDTH") != settings.end())
    {
        VINWIDTH = atoi((settings.at("VINWIDTH")).c_str());
    }
    if (settings.find("VINHEIGHT") != settings.end())
    {
        VINHEIGHT = atoi((settings.at("VINHEIGHT")).c_str());
    }
    if (settings.find("VOUTWIDTH") != settings.end())
    {
        VOUTWIDTH = atoi((settings.at("VOUTWIDTH")).c_str());
    }
    if (settings.find("VOUTHEIGHT") != settings.end())
    {
        VOUTHEIGHT = atoi((settings.at("VOUTHEIGHT")).c_str());
    }
    if (settings.find("PARALLEL") != settings.end())
    {
        PARALLEL = (settings.at("PARALLEL") == "TRUE") ? true : false;
    }
    if (settings.find("YCBCR422") != settings.end())
    {
        YCBCR422 = (settings.at("YCBCR422") == "TRUE") ? true : false;
    }
    if (settings.find("YCBCR444") != settings.end())
    {
        YCBCR444 = (settings.at("YCBCR444") == "TRUE") ? true : false;
    }
    

    if (settings.find("KERNEL") != settings.end())
    {
        strcpy(str_value, (settings.at("KERNEL")).c_str());
       if (strstr(str_value, "LANCZOS") != NULL)
            KERNEL = 4;
    }

    if (settings.find("VFTAPS") != settings.end())
    {
        VFTAPS = atoi((settings.at("VFTAPS")).c_str());
    }
    if (settings.find("HFTAPS") != settings.end())
    {
        HFTAPS = atoi((settings.at("HFTAPS")).c_str());
    }
    if (settings.find("VFPHASES") != settings.end())
    {
        VFPHASES = atoi((settings.at("VFPHASES")).c_str());
    }
    if (settings.find("HFPHASES") != settings.end())
    {
        HFPHASES = atoi((settings.at("HFPHASES")).c_str());
    }
    if (settings.find("DYNAMIC") != settings.end())
    {
        DYNAMIC = (settings.at("DYNAMIC") == "TRUE") ? true : false;
    }
    if (settings.find("ADAPTIVE") != settings.end())
    {
        ADAPTIVE = (settings.at("ADAPTIVE") == "TRUE") ? 1 : 0;
    }
    if (settings.find("SEPPCLK") != settings.end())
    {
        SEPPCLK = (settings.at("SEPPCLK") == "TRUE") ? true : false;
    }

    if (settings.find("CTYPE") != settings.end())
    {
        strcpy(str_value, (settings.at("CTYPE")).c_str());
        if (strstr(str_value, "SIGNED") != NULL)
            COEFF_TYPE = 1;
        else
            COEFF_TYPE = 0;
    }
    

    if (settings.find("RESAMPLE") != settings.end())
    {
        strcpy(str_value, (settings.at("RESAMPLE")).c_str());
        if (strstr(str_value, "AUTO") != NULL)
            RESAMPLE = 2;
        if (strstr(str_value, "AVERAGE") != NULL)
            RESAMPLE = 1;
        else
            RESAMPLE = 0;

    }


    if (settings.find("DWIDTH") != settings.end())
    {
        DIN_WIDTH = atoi((settings.at("DWIDTH")).c_str());
    }
    
    if (settings.find("CWIDTH") != settings.end())
    {
        COEFF_WIDTH = atoi((settings.at("CWIDTH")).c_str());
    }

   /* if (settings.find("MOUTWIDTH") != settings.end())
    {
        MOUTWIDTH = atoi((settings.at("MOUTWIDTH")).c_str());
    }*/
    
    if (settings.find("DOUTWIDTH") != settings.end())
    {
        DOUT_WIDTH = atoi((settings.at("DOUTWIDTH")).c_str());
    }
    
    if (settings.find("PBUSWIDTH") != settings.end())
    {
        PBUSWIDTH = atoi((settings.at("PBUSWIDTH")).c_str());
    }
    

    if (settings.find("LBUFFER") != settings.end())
    {
        strcpy(str_value, (settings.at("LBUFFER")).c_str());
        if (strstr(str_value, "EBR") != NULL)
            LBUFFER = 1;
        else if (strstr(str_value, "DISTRIBUTED") != NULL)
            LBUFFER = 0;
        else
            LBUFFER = 0;
    }


    if (settings.find("VCBUFFER") != settings.end())
    {
        strcpy(str_value, (settings.at("VCBUFFER")).c_str());
        if (strstr(str_value, "EBR") != NULL)
            VCBUFFER = 1;
        else if (strstr(str_value, "DISTRIBUTED") != NULL)
            VCBUFFER = 0;
        else
            VCBUFFER = 0;
    }


    if (settings.find("HCBUFFER") != settings.end())
    {
        strcpy(str_value, (settings.at("HCBUFFER")).c_str());
        if (strstr(str_value, "EBR") != NULL)
            HCBUFFER = 1;
        else if (strstr(str_value, "DISTRIBUTED") != NULL)
            HCBUFFER = 0;
        else
            HCBUFFER = 0;
    }


    if (settings.find("SHARE_CMEM") != settings.end())
    {
        SHARE_CMEM = (settings.at("SHARE_CMEM") == "TRUE") ? 1 : 0;
    }
    
    if (settings.find("MULTTYPE") != settings.end())
    {
        strcpy(str_value, (settings.at("MULTTYPE")).c_str());
        if (strstr(str_value, "DSP") != NULL)
            MULTTYPE = 1;
        else
            MULTTYPE = 0;
    }

    if (settings.find("HIGHSPEED") != settings.end())
    {
        HIGHSPEED = (settings.at("HIGHSPEED") == "TRUE") ? 1 : 0;
    }
    

    if (settings.find("LSB_METHOD") != settings.end())
    {
        strcpy(str_value, (settings.at("LSB_METHOD")).c_str());
        if (strstr(str_value, "TRUNCATION") != NULL)
            LSB_METHOD = 0;
        else if (strstr(str_value, "NORMAL") != NULL)
            LSB_METHOD = 2;
        else
            LSB_METHOD = 4;
    }


    if (settings.find("CBANKS") != settings.end())
    {
        CBANKS = atoi((settings.at("CBANKS")).c_str());
    }
    

    if (settings.find("COEFILE_0") != settings.end())
    {
        strcpy(str_value, (settings.at("COEFILE_0")).c_str());
        strcpy(COEFILES[0], str_value);
        if (strstr(COEFILES[0], "\n") != NULL) {
            char* token = strtok(COEFILES[0], "\n");
            if (token != NULL) {
                strcpy(COEFILES[0], token);
            }
            else {
                // Handle error
                return true;
            }
        }
    }

    
    if (settings.find("COEFILE_1") != settings.end())
    {
        strcpy(str_value, (settings.at("COEFILE_1")).c_str());
        strcpy(COEFILES[1], str_value);
        if (strstr(COEFILES[1], "\n") != NULL) {
            char* token = strtok(COEFILES[1], "\n");
            if (token != NULL) {
                strcpy(COEFILES[1], token);
            }
            else {
                // Handle error
                return true;
            }
        }
    }


    if (settings.find("COEFILE_2") != settings.end())
    {
        strcpy(str_value, (settings.at("COEFILE_2")).c_str());
        strcpy(COEFILES[2], str_value);
        if (strstr(COEFILES[2], "\n") != NULL) {
            char* token = strtok(COEFILES[2], "\n");
            if (token != NULL) {
                strcpy(COEFILES[2], token);
            }
            else {
                // Handle error
                return true;
            }
        }
    }


    if (settings.find("COEFILE_3") != settings.end())
    {
        strcpy(str_value, (settings.at("COEFILE_3")).c_str());
        strcpy(COEFILES[3], str_value);
        if (strstr(COEFILES[3], "\n") != NULL) {
            char* token = strtok(COEFILES[3], "\n");
            if (token != NULL) {
                strcpy(COEFILES[3], token);
            }
            else {
                // Handle error
                return true;
            }
        }
    }

    if (settings.find("FRMPORTS") != settings.end())
    {
        FRMPORTS = (settings.at("FRMPORTS") == "TRUE") ? true : false;
    }

    if (settings.find("RESYNC") != settings.end())
    {
        RESYNC =(settings.at("RESYNC") == "TRUE") ? true : false;
    }

    if (settings.find("SR") != settings.end())
    {
        SR = (settings.at("SR") == "TRUE") ? true : false;
    }

    if (settings.find("CE") != settings.end())
    {
        CE = (settings.at("CE") == "TRUE") ? true : false;
    }
    
    //if (settings.find("TAGS_WIDTH") != settings.end())
    //{
    //    TAGS_WIDTH = atoi((settings.at("TAGS_WIDTH")).c_str());
    //}



    //only for verification
    //frmwidth_0
    if (settings.find("FRMW_0") != settings.end())
    {
        CUR_FRMWIDTH[0] = atoi((settings.at("FRMW_0")).c_str());
    }
    //frmheight_0
    if (settings.find("FRMH_0") != settings.end())
    {
        CUR_FRMHEIGHT[0] = atoi((settings.at("FRMH_0")).c_str());
    }
    //frmwidth_1
    if (settings.find("FRMW_1") != settings.end())
    {
        CUR_FRMWIDTH[1] = atoi((settings.at("FRMW_1")).c_str());
    }
    //frmheight_1
    if (settings.find("FRMH_1") != settings.end())
    {
        CUR_FRMHEIGHT[1] = atoi((settings.at("FRMH_1")).c_str());
    }
    //frmwidth_2
    if (settings.find("FRMW_2") != settings.end())
    {
        CUR_FRMWIDTH[2] = atoi((settings.at("FRMW_2")).c_str());
    }
    //frmheight_2
    if (settings.find("FRMH_2") != settings.end())
    {
        CUR_FRMHEIGHT[2] = atoi((settings.at("FRMH_2")).c_str());
    }

    //outwidth_0
    if (settings.find("OUTW_0") != settings.end())
    {
        CUR_OUTWIDTH[0] = atoi((settings.at("OUTW_0")).c_str());
    }
    //outheight_0
    if (settings.find("OUTH_0") != settings.end())
    {
        CUR_OUTHEIGHT[0] = atoi((settings.at("OUTH_0")).c_str());
    }
    //outwidth_1
    if (settings.find("OUTW_1") != settings.end())
    {
        CUR_OUTWIDTH[1] = atoi((settings.at("OUTW_1")).c_str());
    }
    //outheight_1
    if (settings.find("OUTH_1") != settings.end())
    {
        CUR_OUTHEIGHT[1] = atoi((settings.at("OUTH_1")).c_str());
    }
    //outwidth_2
    if (settings.find("OUTW_2") != settings.end())
    {
        CUR_OUTWIDTH[2] = atoi((settings.at("OUTW_2")).c_str());
    }
    //outheight_2
    if (settings.find("OUTH_2") != settings.end())
    {
        CUR_OUTHEIGHT[2] = atoi((settings.at("OUTH_2")).c_str());
    }

    //EDGE THRESHOLD
    if (settings.find("EDGE_TH") != settings.end())
    {
        EDGE_TH = atoi((settings.at("EDGE_TH")).c_str());
    }
    //FSCALE
    if (settings.find("FSCALE") != settings.end())
    {
        FSCALE = atof((settings.at("FSCALE")).c_str());
    }
    //
    if (settings.find("CSETS") != settings.end())
    {
        CSETS_CS = atoi((settings.at("CSETS")).c_str());
    }
    //
    if (settings.find("SEED") != settings.end())
    {
        SEED = atoi((settings.at("SEED")).c_str());
    }

    file.close();  // close the input file

    if(KERNEL==4){
        if (COEFF_TYPE) COEFF_POINTS = COEFF_WIDTH - 1;
        else           COEFF_POINTS = COEFF_WIDTH;
        MOUT_WIDTH = DIN_WIDTH;// force vout to positive value
    }

    if (DIN_TYPE) {
        max_din = (sint64)pow(2.0, DIN_WIDTH - 1) - 1;
        min_din = -(sint64)pow(2.0, DIN_WIDTH - 1);
    }
    else {
        max_din = (sint64)pow(2.0, DIN_WIDTH) - 1;
        min_din = 0;
    }
    if (COEFF_TYPE) {
        max_coe = (sint64)pow(2.0, COEFF_WIDTH - 1) - 1;
        min_coe = -(sint64)pow(2.0, COEFF_WIDTH - 1);
    }
    else {
        max_coe = (sint64)pow(2.0, COEFF_WIDTH) - 1;
        min_coe = 0;
    }
    if (LSB_METHOD == 1 || LSB_METHOD == 3) {// not support
        LSB_METHOD = 0;
    }
    if (KERNEL < 4)         CBANKS = 1;
    else if (ADAPTIVE == 1) CBANKS = 2;

    //DOUT_WIDTH   = DIN_WIDTH;
    TOP_PAD = (VFTAPS - 1) / 2;
    LEFT_PAD = (HFTAPS - 1) / 2;
    BOTTOM_PAD = VFTAPS - 1 - TOP_PAD;
    RIGHT_PAD = HFTAPS - 1 - LEFT_PAD;
    IMAGE_HEIGHT = VINHEIGHT + TOP_PAD + BOTTOM_PAD;
    IMAGE_WIDTH = VINWIDTH + LEFT_PAD + RIGHT_PAD;
    FOUT_HEIGHT = VOUTHEIGHT;
    FOUT_WIDTH = VOUTWIDTH;
    DOUT_POINTS = DOUT_WIDTH - DIN_WIDTH;//fix the bug of 8-bit to 10-bit
    srand(SEED);

    //actual number of window coeffs
    if (VFPHASES > VOUTHEIGHT) VFCBPWIDTH = log2(VFPHASES);
    else                    VFCBPWIDTH = log2(VOUTHEIGHT);
    if (HFPHASES > VOUTWIDTH)  HFCBPWIDTH = log2(HFPHASES);
    else                    HFCBPWIDTH = log2(VOUTWIDTH);
   
    VFBVALUE = (sint64)1 << VFCBPWIDTH;
    HFBVALUE = (sint64)1 << HFCBPWIDTH;
    VDFACTOR = floor(((double)VFBVALUE * VINHEIGHT) / VOUTHEIGHT);
    HDFACTOR = floor(((double)HFBVALUE * VINWIDTH) / VOUTWIDTH);
    VFCWIDTH = log2(VINHEIGHT) + VFCBPWIDTH;
    HFCWIDTH = log2(VINWIDTH) + HFCBPWIDTH;
    YFULL_WIDTH = DIN_WIDTH + COEFF_WIDTH + log2(VFTAPS);
    XFULL_WIDTH = MOUT_WIDTH + COEFF_WIDTH + log2(HFTAPS);

    if (DYNAMIC == 0) {
        CUR_FRMWIDTH[0] = VINWIDTH - 1;
        CUR_FRMWIDTH[1] = CUR_FRMWIDTH[0];
        CUR_FRMWIDTH[2] = CUR_FRMWIDTH[0];
        CUR_FRMHEIGHT[0] = VINHEIGHT - 1;
        CUR_FRMHEIGHT[1] = CUR_FRMHEIGHT[0];
        CUR_FRMHEIGHT[2] = CUR_FRMHEIGHT[0];
    }
    else {
        if (CUR_FRMWIDTH[0] < 0) CUR_FRMWIDTH[0] = VINWIDTH - 1;
        if (CUR_FRMWIDTH[1] < 0) CUR_FRMWIDTH[1] = CUR_FRMWIDTH[0] - 10;
        if (CUR_FRMWIDTH[2] < 0) CUR_FRMWIDTH[2] = CUR_FRMWIDTH[0] - 20;
        if (CUR_FRMHEIGHT[0] < 0) CUR_FRMHEIGHT[0] = VINHEIGHT - 1;
        if (CUR_FRMHEIGHT[1] < 0) CUR_FRMHEIGHT[1] = CUR_FRMHEIGHT[0] - 10;
        if (CUR_FRMHEIGHT[2] < 0) CUR_FRMHEIGHT[2] = CUR_FRMHEIGHT[0] - 20;
    }
    if (DYNAMIC == 0) {
        CUR_OUTWIDTH[0] = VOUTWIDTH - 1;
        CUR_OUTWIDTH[1] = CUR_OUTWIDTH[0];
        CUR_OUTWIDTH[2] = CUR_OUTWIDTH[0];
        CUR_OUTHEIGHT[0] = VOUTHEIGHT - 1;
        CUR_OUTHEIGHT[1] = CUR_OUTHEIGHT[0];
        CUR_OUTHEIGHT[2] = CUR_OUTHEIGHT[0];
    }
    else {
        if (CUR_OUTWIDTH[0] < 0) CUR_OUTWIDTH[0] = VOUTWIDTH - 1;
        if (CUR_OUTWIDTH[1] < 0) CUR_OUTWIDTH[1] = ((CUR_OUTWIDTH[0] + 1) / 4) * 2 - 1;
        if (CUR_OUTWIDTH[2] < 0) CUR_OUTWIDTH[2] = ((CUR_OUTWIDTH[0] + 1) / 8) * 2 - 1;
        if (CUR_OUTHEIGHT[0] < 0) CUR_OUTHEIGHT[0] = VOUTHEIGHT - 1;
        if (CUR_OUTHEIGHT[1] < 0) CUR_OUTHEIGHT[1] = (CUR_OUTHEIGHT[0] + 1) / 2 - 1;
        if (CUR_OUTHEIGHT[2] < 0) CUR_OUTHEIGHT[2] = (CUR_OUTHEIGHT[0] + 1) / 4 - 1;
    }
    {// must larger than 32
        if (CUR_FRMWIDTH[0] < 31) CUR_FRMWIDTH[0] = 31;
        if (CUR_FRMWIDTH[1] < 31) CUR_FRMWIDTH[1] = 31;
        if (CUR_FRMWIDTH[2] < 31) CUR_FRMWIDTH[2] = 31;
        if (CUR_FRMHEIGHT[0] < 31) CUR_FRMHEIGHT[0] = 31;
        if (CUR_FRMHEIGHT[1] < 31) CUR_FRMHEIGHT[1] = 31;
        if (CUR_FRMHEIGHT[2] < 31) CUR_FRMHEIGHT[2] = 31;
        if (CUR_OUTWIDTH[0] < 31) CUR_OUTWIDTH[0] = 31;
        if (CUR_OUTWIDTH[1] < 31) CUR_OUTWIDTH[1] = 31;
        if (CUR_OUTWIDTH[2] < 31) CUR_OUTWIDTH[2] = 31;
        if (CUR_OUTHEIGHT[0] < 31) CUR_OUTHEIGHT[0] = 31;
        if (CUR_OUTHEIGHT[1] < 31) CUR_OUTHEIGHT[1] = 31;
        if (CUR_OUTHEIGHT[2] < 31) CUR_OUTHEIGHT[2] = 31;
    }
    return false;
}


bool scaler::genparams()
{
   FILE *fh;
   int MAX_TOTAL;
   if(VINWIDTH>VOUTWIDTH)     MAX_TOTAL = VINWIDTH;
   else                       MAX_TOTAL = VOUTWIDTH;
   if(VINHEIGHT>VOUTHEIGHT)   MAX_TOTAL *= VINHEIGHT;
   else                       MAX_TOTAL *= VOUTHEIGHT;
   MAX_TOTAL *= NUM_PLANE*NUM_FRAME;
   if ((fh = fopen("params.v","w")) == NULL) return true;
   fprintf(fh, "parameter SCALER_NAME       = \"%s\";\n",SCALER_NAME);
   fprintf(fh, "parameter NUM_PLANE         = %d;\n", NUM_PLANE);
   fprintf(fh, "parameter DWIDTH            = %d;\n", DIN_WIDTH);
   fprintf(fh, "parameter CWIDTH            = %d;\n", COEFF_WIDTH);
   fprintf(fh, "parameter VINWIDTH          = %d;\n", VINWIDTH);
   fprintf(fh, "parameter VINHEIGHT         = %d;\n", VINHEIGHT);
   fprintf(fh, "parameter VOUTWIDTH         = %d;\n", VOUTWIDTH);
   fprintf(fh, "parameter VOUTHEIGHT        = %d;\n", VOUTHEIGHT);
   if(KERNEL==4) {
   fprintf(fh, "parameter KERNEL            = \"LANCZOS\";\n");
   } else {
   fprintf(fh, "parameter KERNEL            = \"CUSTOM\";\n");
   }
   fprintf(fh, "parameter VFTAPS            = %d;\n", VFTAPS);
   fprintf(fh, "parameter HFTAPS            = %d;\n", HFTAPS);
   fprintf(fh, "parameter VFPHASES          = %d;\n", VFPHASES);
   fprintf(fh, "parameter HFPHASES          = %d;\n", HFPHASES);
   if(PAD_TYPE==0) {
   fprintf(fh, "parameter EDGEMODE          = \"VALUE\";\n");
   } else if(PAD_TYPE==1) {
   fprintf(fh, "parameter EDGEMODE          = \"COPY\";\n");
   } else if(PAD_TYPE==2) {
   fprintf(fh, "parameter EDGEMODE          = \"MIRROR\";\n");
   }
   fprintf(fh, "parameter EDGEVALUE         = %d;\n", PAD_VALUE);
   fprintf(fh, "parameter VSFACTOR          = %d;\n", VDFACTOR);
   fprintf(fh, "parameter HSFACTOR          = %d;\n", HDFACTOR);
   fprintf(fh, "parameter VFCWIDTH          = %d;\n", VFCWIDTH);
   fprintf(fh, "parameter VFCBPWIDTH        = %d;\n", VFCBPWIDTH);
   fprintf(fh, "parameter HFCWIDTH          = %d;\n", HFCWIDTH);
   fprintf(fh, "parameter HFCBPWIDTH        = %d;\n", HFCBPWIDTH);
   if(PARALLEL==1) {
   fprintf(fh, "parameter PARALLEL          = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter PARALLEL          = \"FALSE\";\n");
   }
   if(YCBCR422==1) {
   fprintf(fh, "parameter YCBCR422          = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter YCBCR422          = \"FALSE\";\n");
   }
   if(YCBCR444==1) {
   fprintf(fh, "parameter YCBCR444          = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter YCBCR444          = \"FALSE\";\n");
   }
   if(RESAMPLE==2) {
   fprintf(fh, "parameter RESAMPLE          = \"AUTO\";\n");
   } else if(RESAMPLE==1) {
   fprintf(fh, "parameter RESAMPLE          = \"AVERAGE\";\n");
   } else {
   fprintf(fh, "parameter RESAMPLE          = \"NEAREST\";\n");
   }
   if(DYNAMIC==1) {
   fprintf(fh, "parameter DYNAMIC           = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter DYNAMIC           = \"FALSE\";\n");
   }
   if(ADAPTIVE==1) {
   fprintf(fh, "parameter ADAPTIVE          = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter ADAPTIVE          = \"FALSE\";\n");
   }
   fprintf(fh, "parameter EDGE_TH           = %d;\n", EDGE_TH);
   fprintf(fh, "parameter CBANKS            = %d;\n", CBANKS);
   if(SEPPCLK==1) {
   fprintf(fh, "parameter SEPPCLK           = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter SEPPCLK           = \"FALSE\";\n");
   }
   fprintf(fh, "parameter MOUTWIDTH         = %d;\n", MOUT_WIDTH);
   fprintf(fh, "parameter DOUTWIDTH         = %d;\n", DOUT_WIDTH);
   if(DIN_TYPE) {
   fprintf(fh, "parameter DTYPE             = \"SIGNED\";\n");
   } else {
   fprintf(fh, "parameter DTYPE             = \"UNSIGNED\";\n");
   }
   if(COEFF_TYPE) {
   fprintf(fh, "parameter CTYPE             = \"SIGNED\";\n");
   } else {
   fprintf(fh, "parameter CTYPE             = \"UNSIGNED\";\n");
   }
   fprintf(fh, "parameter OTYPE             = \"UNSIGNED\";\n");
   fprintf(fh, "parameter DIN_POINTS        = %d;\n", DIN_POINTS);
   fprintf(fh, "parameter COEFF_POINTS      = %d;\n", COEFF_POINTS);
   fprintf(fh, "parameter DOUT_POINTS       = %d;\n", DOUT_POINTS);
   fprintf(fh, "parameter PBUSWIDTH         = %d;\n", PBUSWIDTH);
   fprintf(fh, "parameter PADDRWIDTH        = %d;\n", PADDRWIDTH);
   if(LSB_METHOD==4) {
   fprintf(fh, "parameter LSB_METHOD        = \"CONVERGENT\";\n");
   } else if(LSB_METHOD==2) {
   fprintf(fh, "parameter LSB_METHOD        = \"NORMAL\";\n");
   } else {
   fprintf(fh, "parameter LSB_METHOD        = \"TRUNCATION\";\n");
   }
   if(MSB_METHOD==1) {
   fprintf(fh, "parameter MSB_METHOD        = \"WRAP_AROUND\";\n");
   } else {
   fprintf(fh, "parameter MSB_METHOD        = \"SATURATION\";\n");
   }
   if(LBUFFER==1) {
   fprintf(fh, "parameter LBUFFER           = \"EBR\";\n");
   } else {
   fprintf(fh, "parameter LBUFFER           = \"PFU\";\n");
   }
   if(VCBUFFER==1) {
   fprintf(fh, "parameter VCBUFFER          = \"EBR\";\n");
   } else {
   fprintf(fh, "parameter VCBUFFER          = \"PFU\";\n");
   }
   if(HCBUFFER==1) {
   fprintf(fh, "parameter HCBUFFER          = \"EBR\";\n");
   } else {
   fprintf(fh, "parameter HCBUFFER          = \"PFU\";\n");
   }
   if(SHARE_CMEM==1) {
   fprintf(fh, "parameter SHARE_CMEM        = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter SHARE_CMEM        = \"FALSE\";\n");
   }
   if(MULTTYPE==1) {
   fprintf(fh, "parameter MULTTYPE          = \"DSP\";\n");
   } else {
   fprintf(fh, "parameter MULTTYPE          = \"LUT\";\n");
   }
   if(HIGHSPEED==1) {
   fprintf(fh, "parameter HIGHSPEED         = \"TRUE\";\n");
   } else {
   fprintf(fh, "parameter HIGHSPEED         = \"FALSE\";\n");
   }
   fprintf(fh, "parameter FAMILY            = \"%s\";\n",DEVICE);

   fprintf(fh, "// params only for simulation\n");
   fprintf(fh, "parameter NUM_FRAME         = %d;\n", NUM_FRAME);
   fprintf(fh, "parameter SEED              = %d;\n",SEED);
   fprintf(fh, "parameter TOTAL_INPUT       = %d;\n",TOTAL_INPUT);
   fprintf(fh, "parameter TOTAL_OUTPUT      = %d;\n",TOTAL_OUTPUT);
   fprintf(fh, "parameter MAX_TOTAL         = %d;\n",MAX_TOTAL);
   fprintf(fh, "parameter FRMWIDTH_0        = %d;\n",CUR_FRMWIDTH[0]);
   fprintf(fh, "parameter FRMWIDTH_1        = %d;\n",CUR_FRMWIDTH[1]);
   fprintf(fh, "parameter FRMWIDTH_2        = %d;\n",CUR_FRMWIDTH[2]);
   fprintf(fh, "parameter FRMHEIGHT_0       = %d;\n",CUR_FRMHEIGHT[0]);
   fprintf(fh, "parameter FRMHEIGHT_1       = %d;\n",CUR_FRMHEIGHT[1]);
   fprintf(fh, "parameter FRMHEIGHT_2       = %d;\n",CUR_FRMHEIGHT[2]);
   fprintf(fh, "parameter OUTWIDTH_0        = %d;\n",CUR_OUTWIDTH[0]);
   fprintf(fh, "parameter OUTWIDTH_1        = %d;\n",CUR_OUTWIDTH[1]);
   fprintf(fh, "parameter OUTWIDTH_2        = %d;\n",CUR_OUTWIDTH[2]);
   fprintf(fh, "parameter OUTHEIGHT_0       = %d;\n",CUR_OUTHEIGHT[0]);
   fprintf(fh, "parameter OUTHEIGHT_1       = %d;\n",CUR_OUTHEIGHT[1]);
   fprintf(fh, "parameter OUTHEIGHT_2       = %d;\n",CUR_OUTHEIGHT[2]);
   fclose(fh);
   return false;
}
bool scaler::gen_orcapp()
{
   FILE *fh;
   FILE *fhs;
   char line[100];
   int IPWIDTH,OPWIDTH;
   IPWIDTH    = (PARALLEL==0) ? DIN_WIDTH : (YCBCR422==1) ? 2*DIN_WIDTH : NUM_PLANE*DIN_WIDTH;
   OPWIDTH    = (PARALLEL==0) ? DOUT_WIDTH : (YCBCR422==1) ? 2*DOUT_WIDTH : NUM_PLANE*DOUT_WIDTH;

   fh = NULL;
   fh = fopen("orcapp_head","w");
   if (fh == NULL) {
       printf("Can't generate orcapp_head file\n");
       return true;
   }
   fprintf(fh, "#define    ORCAPP_USERNAME            %s\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_u1_USERNAME         u1_%s\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_USERNAME_top        %s_top\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_USERNAME_tb         %s_tb\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_u1_USERNAME_top     u1_%s_top\n",SCALER_NAME);

   fprintf(fh, "#define    ORCAPP_SCALER_NAME        \"%s\"\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_NUM_PLANE          %d\n", NUM_PLANE);
   fprintf(fh, "#define    ORCAPP_DWIDTH             %d\n", DIN_WIDTH);
   fprintf(fh, "#define    ORCAPP_CWIDTH             %d\n", COEFF_WIDTH);
   fprintf(fh, "#define    ORCAPP_VINWIDTH           %d\n", VINWIDTH);
   fprintf(fh, "#define    ORCAPP_VINHEIGHT          %d\n", VINHEIGHT);
   fprintf(fh, "#define    ORCAPP_VOUTWIDTH          %d\n", VOUTWIDTH);
   fprintf(fh, "#define    ORCAPP_VOUTHEIGHT         %d\n", VOUTHEIGHT);
   if(KERNEL==4) {
   fprintf(fh, "#define    ORCAPP_KERNEL             \"LANCZOS\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_KERNEL             \"CUSTOM\"\n");
   }
   fprintf(fh, "#define    ORCAPP_VFTAPS             %d\n", VFTAPS);
   fprintf(fh, "#define    ORCAPP_HFTAPS             %d\n", HFTAPS);
   fprintf(fh, "#define    ORCAPP_VFPHASES           %d\n", VFPHASES);
   fprintf(fh, "#define    ORCAPP_HFPHASES           %d\n", HFPHASES);
   if(PAD_TYPE==0) {
   fprintf(fh, "#define    ORCAPP_EDGEMODE           \"VALUE\"\n");
   } else if(PAD_TYPE==1) {
   fprintf(fh, "#define    ORCAPP_EDGEMODE           \"COPY\"\n");
   } else if(PAD_TYPE==2) {
   fprintf(fh, "#define    ORCAPP_EDGEMODE           \"MIRROR\"\n");
   }
   fprintf(fh, "#define    ORCAPP_EDGEVALUE          %d\n", PAD_VALUE);
   fprintf(fh, "#define    ORCAPP_VSFACTOR           %d\n", VDFACTOR);
   fprintf(fh, "#define    ORCAPP_HSFACTOR           %d\n", HDFACTOR);
   fprintf(fh, "#define    ORCAPP_VFCWIDTH           %d\n", VFCWIDTH);
   fprintf(fh, "#define    ORCAPP_VFCBPWIDTH         %d\n", VFCBPWIDTH);
   fprintf(fh, "#define    ORCAPP_HFCWIDTH           %d\n", HFCWIDTH);
   fprintf(fh, "#define    ORCAPP_HFCBPWIDTH         %d\n", HFCBPWIDTH);
   if(PARALLEL==1) {
   fprintf(fh, "#define    ORCAPP_PARALLEL           \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_PARALLEL           \"FALSE\"\n");
   }
   if(YCBCR422==1) {
   fprintf(fh, "#define    ORCAPP_YCBCR422           \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_YCBCR422           \"FALSE\"\n");
   }
   if(YCBCR444==1) {
   fprintf(fh, "#define    ORCAPP_YCBCR444           \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_YCBCR444           \"FALSE\"\n");
   }
   if(RESAMPLE==2) {
   fprintf(fh, "#define    ORCAPP_RESAMPLE           \"AUTO\"\n");
   } else if(RESAMPLE==1) {
   fprintf(fh, "#define    ORCAPP_RESAMPLE           \"AVERAGE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_RESAMPLE           \"NEAREST\"\n");
   }
   if(DYNAMIC==1) {
   fprintf(fh, "#define    ORCAPP_DYNAMIC            \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_DYNAMIC            \"FALSE\"\n");
   }
   if(ADAPTIVE==1) {
   fprintf(fh, "#define    ORCAPP_ADAPTIVE           \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_ADAPTIVE           \"FALSE\"\n");
   }
   fprintf(fh, "#define    ORCAPP_EDGE_TH            %d\n", EDGE_TH);
   fprintf(fh, "#define    ORCAPP_CBANKS             %d\n", CBANKS);
   if(SEPPCLK==0) {
   fprintf(fh, "#define    ORCAPP_SEPPCLK             \"FALSE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_SEPPCLK             \"TRUE\"\n");
   }
   fprintf(fh, "#define    ORCAPP_MOUTWIDTH          %d\n", MOUT_WIDTH);
   fprintf(fh, "#define    ORCAPP_DOUTWIDTH          %d\n", DOUT_WIDTH);
   if(DIN_TYPE) {
   fprintf(fh, "#define    ORCAPP_DTYPE              \"SIGNED\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_DTYPE              \"UNSIGNED\"\n");
   }
   if(COEFF_TYPE) {
   fprintf(fh, "#define    ORCAPP_CTYPE              \"SIGNED\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_CTYPE              \"UNSIGNED\"\n");
   }
   fprintf(fh, "#define    ORCAPP_OTYPE              \"UNSIGNED\"\n");
   fprintf(fh, "#define    ORCAPP_DIN_POINTS         %d\n", DIN_POINTS);
   fprintf(fh, "#define    ORCAPP_COEFF_POINTS       %d\n", COEFF_POINTS);
   fprintf(fh, "#define    ORCAPP_DOUT_POINTS        %d\n", DOUT_POINTS);
   fprintf(fh, "#define    ORCAPP_PBUSWIDTH          %d\n", PBUSWIDTH);
   fprintf(fh, "#define    ORCAPP_PADDRWIDTH         %d\n", PADDRWIDTH);
   if(LSB_METHOD==4) {
   fprintf(fh, "#define    ORCAPP_LSB_METHOD         \"CONVERGENT\"\n");
   } else if(LSB_METHOD==2) {
   fprintf(fh, "#define    ORCAPP_LSB_METHOD         \"NORMAL\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_LSB_METHOD         \"TRUNCATION\"\n");
   }
   if(MSB_METHOD==1) {
   fprintf(fh, "#define    ORCAPP_MSB_METHOD         \"WRAP_AROUND\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_MSB_METHOD         \"SATURATION\"\n");
   }
   if(LBUFFER==1) {
   fprintf(fh, "#define    ORCAPP_LBUFFER            \"EBR\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_LBUFFER            \"PFU\"\n");
   }
   if(VCBUFFER==1) {
   fprintf(fh, "#define    ORCAPP_VCBUFFER           \"EBR\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_VCBUFFER           \"PFU\"\n");
   }
   if(HCBUFFER==1) {
   fprintf(fh, "#define    ORCAPP_HCBUFFER           \"EBR\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_HCBUFFER           \"PFU\"\n");
   }
   if(SHARE_CMEM==1) {
   fprintf(fh, "#define    ORCAPP_SHARE_CMEM         \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_SHARE_CMEM         \"FALSE\"\n");
   }
   if(MULTTYPE==1) {
   fprintf(fh, "#define    ORCAPP_MULTTYPE           \"DSP\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_MULTTYPE           \"LUT\"\n");
   }
   if(HIGHSPEED==1) {
   fprintf(fh, "#define    ORCAPP_HIGHSPEED          \"TRUE\"\n");
   } else {
   fprintf(fh, "#define    ORCAPP_HIGHSPEED          \"FALSE\"\n");
   }
   fprintf(fh, "#define    ORCAPP_FAMILY             \"%s\"\n",DEVICE);

   // for orcapp ports
   fprintf(fh, "#define    ORCAPP_IPWIDTH             %d\n", IPWIDTH);
   fprintf(fh, "#define    ORCAPP_OPWIDTH             %d\n", OPWIDTH);
   if(DYNAMIC==1 && SEPPCLK==1) {
   fprintf(fh, "#define    ORCAPP_PCLK\n");
   }
   if(DYNAMIC==1) {
   fprintf(fh, "#define    ORCAPP_DYNAMIC_PORTS      \n");
   }
   fprintf(fh, "#define    ORCAPP_DEVICE          DEVICE_%s\n",DEVICE);
   fprintf(fh, "#define    ORCAPP_STIMULUS        \"stimulus_%s.dat\"\n",SCALER_NAME);
   fprintf(fh, "#define    ORCAPP_GOLDEN          \"golden_%s.dat\"\n",SCALER_NAME);

   fprintf(fh, "#define    ORCAPP_OXWIDTH             %d\n", log2(VOUTWIDTH));
   fprintf(fh, "#define    ORCAPP_OYWIDTH             %d\n", log2(VOUTHEIGHT));
   if(FRMPORTS==1) {
   fprintf(fh, "#define    ORCAPP_FRMPORTS\n");
   }
   if(RESYNC==1) {
   fprintf(fh, "#define    ORCAPP_RESYNC\n");
   }
   if (SR == 1) fprintf(fh,"#define    ORCAPP_PORTSR\n");
   if (CE == 1) fprintf(fh,"#define    ORCAPP_PORTCE\n");
   fclose(fh);
   return false;
}

int scaler::allocate_mem()
{
   int ii,jj,kk;
   // coefficients memory
   vcmem = new int **[MAX_CBANKS];
   hcmem = new int **[MAX_CBANKS];
   for(kk=0;kk<MAX_CBANKS;kk++) {
      vcmem[kk] = new int *[VFPHASES];
      if(vcmem[kk]==NULL) return 1;
      for (ii=0;ii<VFPHASES;ii++) {
         vcmem[kk][ii] = new int [VFTAPS];
         if(vcmem[kk][ii]==NULL) return 1;
      }
      hcmem[kk] = new int *[HFPHASES];
      if(hcmem==NULL) return 1;
      for (ii=0;ii<HFPHASES;ii++) {
         hcmem[kk][ii] = new int [HFTAPS];
         if(hcmem[kk][ii]==NULL) return 1;
      }
      for (ii=0;ii<VFPHASES;ii++) {
         for (jj=0;jj<VFTAPS;jj++) {
            vcmem[kk][ii][jj] = 0;
         }
      }
      for (ii=0;ii<HFPHASES;ii++) {
         for (jj=0;jj<HFTAPS;jj++) {
            hcmem[kk][ii][jj] = 0;
         }
      }
   }

   // stimulus memory
   dmem = new int **[NUM_PLANE];
   if(dmem==NULL) return 2;
   for (ii=0;ii<NUM_PLANE;ii++) {
      dmem[ii] = new int *[IMAGE_HEIGHT];
      if(dmem[ii]==NULL) return 2;
      for (jj=0;jj<IMAGE_HEIGHT;jj++) {
         dmem[ii][jj] = new int [IMAGE_WIDTH];
         if(dmem[ii][jj]==NULL) return 2;
      }
   }
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<IMAGE_HEIGHT;jj++) {
         for (kk=0;kk<IMAGE_WIDTH;kk++) {
            dmem[ii][jj][kk] = 0;
         }
      }
   }
   //middle memory for vertical scaling only
   hmem = new int **[NUM_PLANE];
   if(hmem==NULL) return 2;
   for (ii=0;ii<NUM_PLANE;ii++) {
      hmem[ii] = new int *[FOUT_HEIGHT];
      if(hmem[ii]==NULL) return 2;
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         hmem[ii][jj] = new int [IMAGE_WIDTH];
         if(hmem[ii][jj]==NULL) return 2;
      }
   }
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         for (kk=0;kk<IMAGE_WIDTH;kk++) {
            hmem[ii][jj][kk] = 0;
         }
      }
   }
   // golden memory
   gmem = new int **[NUM_PLANE];
   if(gmem==NULL) return 3;
   for (ii=0;ii<NUM_PLANE;ii++) {
      gmem[ii] = new int*[FOUT_HEIGHT];
      if(gmem[ii]==NULL) return 3;
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         gmem[ii][jj] = new int[FOUT_WIDTH];
         if(gmem[ii][jj]==NULL) return 3;
      }
   }
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         for (kk=0;kk<FOUT_WIDTH;kk++) {
            gmem[ii][jj][kk] = 0;
         }
      }
   }
   return 0;
}
scaler::~scaler()
{
   int ii,jj,kk;

   // delete memory
   for (kk=0;kk<MAX_CBANKS;kk++) {
      for (ii=0;ii<VFPHASES;ii++) {
         delete [] vcmem[kk][ii];
         vcmem[kk][ii] = NULL;
      }
      delete [] vcmem[kk];
      vcmem[kk] = NULL;
   }
   delete [] vcmem;
   vcmem = NULL;

   for (kk=0;kk<MAX_CBANKS;kk++) {
      for (ii=0;ii<HFPHASES;ii++) {
         delete [] hcmem[kk][ii];
         hcmem[kk][ii] = NULL;
      }
      delete [] hcmem[kk];
      hcmem[kk] = NULL;
   }
   delete [] hcmem;
   hcmem = NULL;

   // delete stimulus memory
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<IMAGE_HEIGHT;jj++) {
         delete [] dmem[ii][jj];
         dmem[ii][jj] = NULL;
      }
      delete [] dmem[ii];
      dmem[ii] = NULL;
   }
   delete [] dmem;
   dmem = NULL;
   // delete middle memory
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         delete [] hmem[ii][jj];
         hmem[ii][jj] = NULL;
      }
      delete [] hmem[ii];
      hmem[ii] = NULL;
   }
   delete [] hmem;
   hmem = NULL;
   // delete golden memory
   for (ii=0;ii<NUM_PLANE;ii++) {
      for (jj=0;jj<FOUT_HEIGHT;jj++) {
         delete [] gmem[ii][jj];
         gmem[ii][jj] = NULL;
      }
      delete [] gmem[ii];
      gmem[ii] = NULL;
   }
   delete [] gmem;
   gmem = NULL;
}

bool scaler::rerand()
{
   int  stime;
   long ltime;
   FILE *fh;
   ltime = time(NULL);
   stime = (unsigned)ltime/2;
   SEED = stime%1000;
   srand(stime);
   fh = fopen("seed.txt","w");
   fprintf(fh,"%d",stime);
   fclose(fh);
   return 0;
}

int scaler::gen_coefficient()
{
   double vfscale,hfscale;
   int FINHEIGHT,FINWIDTH;
   FINHEIGHT = VINHEIGHT;
   FINWIDTH  = VINWIDTH;
   if(DYNAMIC || SHARE_CMEM)                    vfscale = 1;
   else if(FINHEIGHT>VOUTHEIGHT)                vfscale = (double)VOUTHEIGHT/FINHEIGHT;
   else                                         vfscale = 1;
   if(DYNAMIC || SHARE_CMEM)                    hfscale = 1;
   else if(FINWIDTH>VOUTWIDTH)                  hfscale = (double)VOUTWIDTH/FINWIDTH;
   else                                         hfscale = 1;
   if(KERNEL<=4) {
      gen_coeff_from_kernel(vcmem[0],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, vfscale);
      gen_coeff_from_kernel(hcmem[0],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, hfscale);
      if(KERNEL==4 && CBANKS==2) {// for edge adaptive scaling algorithm
         gen_coeff_from_kernel(vcmem[0],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, 1);
         gen_coeff_from_kernel(hcmem[0],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, 1);
         gen_coeff_from_kernel(vcmem[1],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, FSCALE);
         gen_coeff_from_kernel(hcmem[1],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, FSCALE);
      }
   } else if(KERNEL==5) { // custom kernel only
      if (strlen(COEFILES[0]) > 2) {
         gen_coeff_from_file(vcmem[0],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, 0);
         gen_coeff_from_file(hcmem[0],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, 0);
      } else { // default
         gen_coeff_from_kernel(vcmem[0],4,VFTAPS, VFPHASES, COEFF_POINTS, vfscale);
         gen_coeff_from_kernel(hcmem[0],4,HFTAPS, HFPHASES, COEFF_POINTS, hfscale);
      }
      if (strlen(COEFILES[1]) > 2 && CBANKS > 1) {
         gen_coeff_from_file(vcmem[1],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, 1);
         gen_coeff_from_file(hcmem[1],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, 1);
      } else { // default
         gen_coeff_from_kernel(vcmem[1],4,VFTAPS, VFPHASES, COEFF_POINTS, vfscale);
         gen_coeff_from_kernel(hcmem[1],4,HFTAPS, HFPHASES, COEFF_POINTS, hfscale);
      }
      if (strlen(COEFILES[2]) > 2 && CBANKS > 2) {
         gen_coeff_from_file(vcmem[2],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, 2);
         gen_coeff_from_file(hcmem[2],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, 2);
      } else { // default
         gen_coeff_from_kernel(vcmem[2],4,VFTAPS, VFPHASES, COEFF_POINTS, vfscale);
         gen_coeff_from_kernel(hcmem[2],4,HFTAPS, HFPHASES, COEFF_POINTS, hfscale);
      }
      if (strlen(COEFILES[3]) > 2 && CBANKS > 3) {
         gen_coeff_from_file(vcmem[3],KERNEL,VFTAPS, VFPHASES, COEFF_POINTS, 3);
         gen_coeff_from_file(hcmem[3],KERNEL,HFTAPS, HFPHASES, COEFF_POINTS, 3);
      } else { // default
         gen_coeff_from_kernel(vcmem[3],4,VFTAPS, VFPHASES, COEFF_POINTS, vfscale);
         gen_coeff_from_kernel(hcmem[3],4,HFTAPS, HFPHASES, COEFF_POINTS, hfscale);
      }
   }
   return false;
}
int scaler::gen_coeff_from_kernel(int **coemem,int kernel,int taps,int phases,int cpoints,double fscale)
{
   double *phs;
   double ph,sum,x;
   int coe;
   int ii,jj;
   int mm,tmp,sum1;
   double N = (taps+1)/2.0;
   phs = new double[taps+10];
   for(ii=0;ii<phases;ii++) {
      sum     = 0;
      ph      = (double)ii/phases;
      for(jj=0;jj<taps;jj++) {
         if(jj<taps/2)
            x = (taps/2 - 1 - jj) + ph;
         else
            x = (jj - taps/2 + 1) - ph;
         phs[jj] = coeff_kernel(kernel,N,x*fscale)*fscale;
         sum += phs[jj];
      }
      for(jj=0;jj<taps;jj++) {
         phs[jj] = (phs[jj]/sum)*((1<<cpoints)-1);
         coe = floor(phs[jj]+0.5);
         if (coe > max_coe) coe = max_coe;
         if (coe < min_coe) coe = min_coe;
         coemem[ii][jj] = coe;
      }
      if(KERNEL>1) { // make sure all the phases have the same sum value
         tmp = coemem[ii][0];
         mm  = 0;
         sum1= 0;
         for(jj=0;jj<taps;jj++) {
            if(COEFF_TYPE==true && coemem[ii][jj] < tmp) {
               tmp = coemem[ii][jj];
               mm  = jj;
            }
            if(COEFF_TYPE==false && coemem[ii][jj] > tmp) {
               tmp = coemem[ii][jj];
               mm  = jj;
            }
            sum1 = sum1 + coemem[ii][jj];
         }
         if(sum1 != ((1<<cpoints)-1)) {
            coemem[ii][mm] += ((1<<cpoints)-1) - sum1;
         }
      }
   }

   delete [] phs;
   return 0;
}

double scaler::coeff_kernel(int kernel, double N, double x)
{
   double f,absx,absx2,absx3,pi,pix,pix2;
   double p0,p1,p2,p3,q0,q1,q2,q3;
   int n;
   if(x<0) absx = -1*x;
   else    absx = x;
   absx2 = absx*absx;
   absx3 = absx*absx2;
   pi    = 3.1415926535897932;
   pix   = pi*x;
   pix2  = pix*pix;
   f     = 0;

   if(kernel==4) { // lanczosN
      if(x==0)
         f = 1;
      else if(absx < N)
         f = (sin(pix) * sin(pix/N))/(pix2/N);
   }
   if(COEFF_TYPE==false && f<0) 
      f = 0; // set min coeff value to zero when unsigned format
   return f;
}

int scaler::gen_coeff_from_file(int **coemem,int kernel,int taps,int phases,int cpoints,int cset)
{
   FILE *fh;
   double *phs;
   double ph,sum,x;
   double coes[20];
   int coe,N;
   int ii,jj;
   int mm,tmp,sum1;
   N = (taps+1)/2;
   fh = fopen(COEFILES[cset],"r");
   if (fh == NULL) {
       printf("Can't open %s for read\n",COEFILES[cset]);
       return true;
   }
   for(ii=0;ii<phases;ii++) {
      if(taps==4) {
         fscanf(fh,"%lf%lf%lf%lf",                        &coes[0],&coes[1],&coes[2],&coes[3]);
      } else if(taps==5) {
         fscanf(fh,"%lf%lf%lf%lf%lf",                     &coes[0],&coes[1],&coes[2],&coes[3],&coes[4]);
      } else if(taps==6) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf",                  &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5]);
      } else if(taps==7) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf",               &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6]);
      } else if(taps==8) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf%lf",            &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6],&coes[7]);
      } else if(taps==9) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf%lf%lf",         &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6],&coes[7],&coes[8]);
      } else if(taps==10) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",      &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6],&coes[7],&coes[8],&coes[9]);
      } else if(taps==11) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",   &coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6],&coes[7],&coes[8],&coes[9],&coes[10]);
      } else if(taps==12) {
         fscanf(fh,"%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",&coes[0],&coes[1],&coes[2],&coes[3],&coes[4],&coes[5],&coes[6],&coes[7],&coes[8],&coes[9],&coes[10],&coes[11]);
      }
      sum     = 0;
      for(jj=0;jj<taps;jj++) {
         sum += coes[jj];
      }
      for(jj=0;jj<taps;jj++) {
         coes[jj] = (coes[jj]/sum)*(1<<cpoints);
         coe = floor(coes[jj]+0.5);
         if (coe > max_coe) coe = max_coe;
         if (coe < min_coe) coe = min_coe;
         coemem[ii][jj] = coe;
      }
      if(KERNEL>1) { // make sure all the phases have the same sum value
         tmp = coemem[ii][0];
         mm  = 0;
         sum1= 0;
         for(jj=0;jj<taps;jj++) {
            if(COEFF_TYPE==true && coemem[ii][jj] < tmp) {
               tmp = coemem[ii][jj];
               mm  = jj;
            }
            if(COEFF_TYPE==false && coemem[ii][jj] > tmp) {
               tmp = coemem[ii][jj];
               mm  = jj;
            }
            sum1 = sum1 + coemem[ii][jj];
         }
         if(sum1 != ((1<<cpoints)-1)) {
            coemem[ii][mm] += ((1<<cpoints)-1) - sum1;
         }
      }
   }
   fclose(fh);

   return 0;
}




bool scaler::rgb2ycbcr(vector<vector<vector<int> > > &dmem_vect,vector<vector<vector<int> > > &ycbcr_vect)
{

    for (int row = 0; row < dmem_vect[0].size(); row++) {
        for (int col = 0; col < dmem_vect[0][0].size(); col++) {
            int temp = dmem_vect[0][row][col];
            dmem_vect[0][row][col] = dmem_vect[2][row][col];
            dmem_vect[2][row][col] = temp;
        }
    }

    const int num_channels = dmem_vect.size();
    const int height = dmem_vect[0].size();
    const int width = dmem_vect[0][0].size();
    const int num_pixels = height * width;
    // Reshape the dmem_vect data to a 2D matrix
    vector<vector<int> > rgb_data(num_pixels, vector<int>(num_channels,0));

    for (int channel = 0; channel < num_channels; channel++) {
        for (int col = 0; col < dmem_vect[0][0].size(); col++) {
            for (int row = 0; row < dmem_vect[0].size(); row++) {
                const int index = col * dmem_vect[0].size() + row;
                rgb_data[index][channel] = dmem_vect[channel][row][col];
            }
        }
    }
     
    // Define constants
    const double Kb = 0.114;
    const double Kr = 0.299;

    // Define conversion matrix
    vector<vector<double> > cmat(3, vector<double>(3,0.0));
    cmat[0][0] = Kr;
    cmat[0][1] = 1 - Kr - Kb;
    cmat[0][2] = Kb;
    cmat[1][0] = -Kr / (2 - 2 * Kb);
    cmat[1][1] = -(1 - Kr - Kb) / (2 - 2 * Kb);
    cmat[1][2] = 0.5;
    cmat[2][0] = 0.5;
    cmat[2][1] = -(1 - Kr - Kb) / (2 - 2 * Kr);
    cmat[2][2] = -Kb / (2 - 2 * Kr);


    // Compute the product of rgb and cmat transpose
    std::vector<std::vector<double> > out(rgb_data.size(), std::vector<double>(cmat.size(),0.0));
    for (int i = 0; i < rgb_data.size(); i++) {
        for (int j = 0; j < cmat.size(); j++) {
            double sum = 0;
            for (int k = 0; k < rgb_data[0].size(); k++) {
                sum += (static_cast<double>(rgb_data[i][k])/255.0) * cmat[j][k];
            }
            out[i][j] = sum;
        }
    }

    // Add 0.5 to columns 2 and 3 of out
    for (int i = 0; i < out.size(); i++) {
        out[i][1] += 0.5;
        out[i][2] += 0.5;
    }

    // Multiply column 1 by 219/255 and add 16/255
    for (int i = 0; i < out.size(); i++) {
        out[i][0] = (out[i][0] * 219.0 / 255.0) + 16.0 / 255.0;
    }

    // Multiply columns 2 and 3 by 224/255 and add 16/255
    for (int i = 0; i < out.size(); i++) {
        out[i][1] = (out[i][1] * 224.0 / 255.0) + 16.0 / 255.0;
        out[i][2] = (out[i][2] * 224.0 / 255.0) + 16.0 / 255.0;
    }

    // Define a matrix to store the converted data
    std::vector<std::vector<int> > out_uint8(out.size(), std::vector<int>(out[0].size(),0));

    // Convert the data to int 
    for (int i = 0; i < out.size(); i++) {
        for (int j = 0; j < out[0].size(); j++) {
            double val = out[i][j] * 255;
            out_uint8[i][j] = static_cast<int>(round(val));
        }
    }
   
    // Define the dimensions of the output image
    const int nRows = height;
    const int nCols = width;
    std::vector<std::vector<std::vector<int> > > out_image(num_channels, std::vector<std::vector<int> >(height, std::vector<int>(width,0)));

    for (int channel = 0; channel < num_channels; channel++) {
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                const int index = col * height + row;
                out_image[channel][row][col] = out_uint8[index][channel];
            }
        }
    }

    for (int row = 0; row < out_image[0].size(); row++) {
        for (int col = 0; col < out_image[0][0].size(); col++) {
            int temp = out_image[0][row][col];
            out_image[0][row][col] = out_image[2][row][col];
            out_image[2][row][col] = temp;
        }
    }

    ycbcr_vect = out_image;
    return false;
}


bool scaler::gen_stimulus_from_bmp(FILE * fhw)
{
   char fline[100];
   int96   result;
   int     *hexdata;
   int     num_pixel;
   int     idx,lidx;
   int     ii,jj,kk,mm,nn,pp,cc;
   sint64  valued;
   unsigned char     pixel;
   unsigned short int *frm;
   int     IMAGEH,IMAGEW,PLANES,NN,CLINE;
   int     fwidth, fheight;
   int     temp;

   FILE *fhr;


   FILE* fh_C1;
   FILE* fh_C2;
   FILE* fh_C3;

   const char* dirname = "testbench"; // directory name
   int res;
   struct stat st;

   if (stat(dirname, &st) == 0) {
       if (st.st_mode & S_IFDIR) {
           // directory exists
       }
       else {
           // not a directory, handle error
           // ...
       }
   }
   else{
#ifdef PLATFORM_NT
       res = _mkdir(dirname); // create directory with full permissions
       if (res != 0) {
           printf("Can't create testbench directory");
           return true;
       }
#else
       res = mkdir(dirname, 0777); // create directory with full permissions
       if (res != 0) {
           printf("Can't create testbench directory");
           return true;
       }
#endif
   }

   if (YCBCR422 == 0 && YCBCR444 == 0){
       fh_C1 = fopen("testbench/input_file_blue.txt", "wb");
       if (fh_C1 == NULL) return 1;
       fh_C2 = fopen("testbench/input_file_green.txt", "wb");
       if (fh_C2 == NULL) return 1;
       fh_C3 = fopen("testbench/input_file_red.txt", "wb");
       if (fh_C3 == NULL) return 1;
   }
   else{
       fh_C1 = fopen("testbench/input_file_y.txt", "wb");
       if (fh_C1 == NULL) return 1;
       fh_C2 = fopen("testbench/input_file_cb.txt", "wb");
       if (fh_C2 == NULL) return 1;
       fh_C3 = fopen("testbench/input_file_cr.txt", "wb");
       if (fh_C3 == NULL) return 1;
   }

   idx = 0;
   hexdata = new int[(DIN_WIDTH+3)/4+1];
   fhr = fopen(STIMULUSFILE,"rb");
   if (fhr == NULL) {
       printf("Can't open %s for read\n",STIMULUSFILE);
       return true;
   }
   fread(&bfinfo.ID, 1,2,fhr);
   fread(&bfinfo.FS, 1,4,fhr);
   fread(&bfinfo.RS0,1,4,fhr);
   fread(&bfinfo.BDO,1,4,fhr);
   fread(&bfinfo.BHS,1,4,fhr);
   fread(&bfinfo.IMW,1,4,fhr);
   fread(&bfinfo.IMH,1,4,fhr);
   fread(&bfinfo.PS, 1,2,fhr);
   fread(&bfinfo.BPP,1,2,fhr);
   fread(&bfinfo.CPN,1,4,fhr);
   fread(&bfinfo.BDS,1,4,fhr);
   fread(&bfinfo.HRL,1,4,fhr);
   fread(&bfinfo.VRL,1,4,fhr);
   fread(&bfinfo.CS, 1,4,fhr);
   fread(&bfinfo.ICS,1,4,fhr);
   if(bfinfo.BDO<54) bfinfo.BDO = 54;
   NN = (bfinfo.BDO-54)/4;
   bfinfo.CP = new unsigned int [NN+4];
   fread(bfinfo.CP,4,NN,fhr);
   IMAGEW = bfinfo.IMW;
   IMAGEH = bfinfo.IMH;
   PLANES = bfinfo.BPP/8;
   CLINE  = ((IMAGEW*PLANES+3)/4)*4;
   num_pixel = IMAGEH*CLINE;

   vector<vector<vector<int> > > dmem_vect(NUM_PLANE, vector<vector<int> >(VINHEIGHT + TOP_PAD, vector<int>(VINWIDTH + LEFT_PAD, 0)));

   while (idx < num_pixel) {
       fread(&pixel, 1, 1, fhr);
       valued = pixel;
       if (valued > max_din) valued = max_din;
       if (valued < min_din) valued = min_din;
       result = int642int96(valued);
       int962hex(result, hexdata, DIN_WIDTH);
       lidx = idx % CLINE;
       pp = lidx % PLANES;
       jj = IMAGEH - 1 - (idx / CLINE);
       ii = lidx / PLANES;
       if (jj < VINHEIGHT && ii < VINWIDTH) {
           //for (kk=(DIN_WIDTH+3)/4-1;kk>=0;kk--) fprintf(fhw,"%X",hexdata[kk]);
           //fprintf(fhw,"\n");
           mm = (jj + TOP_PAD);
           nn = (ii + LEFT_PAD);
           dmem[pp][mm][nn] = valued;
           dmem_vect[pp][mm][nn] = valued;
           //dmem_vect[pp][jj][ii] = valued;
       }
       idx += 1;
   }



   if (YCBCR422 == 1) {   

       //RGB2YCBCR conversion
       vector<vector<vector<int> > > ycbcr_image(NUM_PLANE, vector<vector<int> >(VINHEIGHT + TOP_PAD, vector<int>(VINWIDTH + LEFT_PAD, 0)));
       rgb2ycbcr(dmem_vect, ycbcr_image);

       int num_planes = ycbcr_image.size();
       int height = ycbcr_image[0].size();
       int width = ycbcr_image[0][0].size();

       for (int i = 0; i < num_planes; i++) {
           for (int j = 0; j < height; j++) {
               for (int k = 0; k < width; k++) {
                   dmem[i][j][k] = ycbcr_image[i][j][k];
               }
           }
       }

       //y
       for (ii = 0; ii < VINHEIGHT; ii++) {
           for (jj = 0; jj < VINWIDTH; jj++) {
               //mm = (ii + TOP_PAD);
               //nn = (jj + LEFT_PAD);
               temp = dmem[0][ii + TOP_PAD][jj + LEFT_PAD];
               result = int642int96(sint64(temp));
               int962hex(result, hexdata, DIN_WIDTH);
               for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C1, "%X", hexdata[kk]);
               fprintf(fh_C1, "\n");

           }
       }

       //cb
       for (ii = 0; ii < VINHEIGHT; ii++) {
           for (jj = 0; jj < VINWIDTH / 2; jj++) {
               //mm = (ii+TOP_PAD);
               //nn = (jj+LEFT_PAD);
               //temp = dmem[1][mm][nn * 2 + 1];
               temp = dmem[1][ii + TOP_PAD][jj * 2 + LEFT_PAD];

               result = int642int96(sint64(temp));
               int962hex(result, hexdata, DIN_WIDTH);
               for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C2, "%X", hexdata[kk]);
               fprintf(fh_C2, "\n");
           }
       }

       //cr
       for (ii = 0; ii < VINHEIGHT; ii++) {
           for (jj = 0; jj < VINWIDTH / 2; jj++) {
               //mm = (ii + TOP_PAD);
               //nn = (jj + LEFT_PAD);
               temp = dmem[2][ii + TOP_PAD][jj * 2 + LEFT_PAD];

               result = int642int96(sint64(temp));
               int962hex(result, hexdata, DIN_WIDTH);
               for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C3, "%X", hexdata[kk]);
               fprintf(fh_C3, "\n");
           }
       }
   }
      
      // [0]->B;[1]->G;[2]->R
      frm = new unsigned short int[VINWIDTH * VINHEIGHT];
   for (cc = 0; cc < NUM_PLANE; cc++) {
       for (ii = 0; ii < VINHEIGHT; ii++) {
           for (jj = 0; jj < VINWIDTH; jj++) {
               mm = (ii + TOP_PAD);
               nn = (jj + LEFT_PAD);
               temp = dmem[cc][mm][nn];

               if (YCBCR422 == 0)
               {
                   result = int642int96(sint64(temp));
                   int962hex(result, hexdata, DIN_WIDTH);
                   if (cc == 0) //Blue
                   {
                       for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C1, "%X", hexdata[kk]);
                       fprintf(fh_C1, "\n");
                   }
                   else if (cc == 1) //Green
                   {
                       for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C2, "%X", hexdata[kk]);
                       fprintf(fh_C2, "\n");
                   }
                   else if (cc == 2) //Red
                   {
                       for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C3, "%X", hexdata[kk]);
                       fprintf(fh_C3, "\n");
                   }
               }
               frm[ii * VINWIDTH + jj] = (unsigned short int)temp;
           }
       }
       //fwrite(frm,2,VINWIDTH*VINHEIGHT,fhw);
       TOTAL_INPUT += VINWIDTH * VINHEIGHT;
   }
   fclose(fhr);
   fclose(fh_C1);
   fclose(fh_C2);
   fclose(fh_C3);

   delete [] hexdata;
   delete [] frm;

   return false;
}

bool scaler::gen_stimulus_from_file(FILE * fhw)
{
   char fline[100];
   int96   result;
   int     *hexdata;
   int     num_pixel;
   int     site;
   int     idx;
   string  line;
   int     ii,jj,kk,mm,nn;
   sint64  valued;
   double  pixel;

   FILE *fhr;

   idx = 0;
   hexdata = new int[(DIN_WIDTH+3)/4+1];
   num_pixel = NUM_PLANE*VINWIDTH*VINHEIGHT;
   fhr = fopen(STIMULUSFILE,"r");
   if (fhr == NULL) {
       printf("Can't open %s for read\n",STIMULUSFILE);
       return true;
   }
   while (idx < num_pixel) {
      strcpy(fline,"");
      line.empty();
      if (feof(fhr)) { break; }
      fgets(fline,100,fhr);
      line = fline;
      site = line.find('/');
      if (site >= 0) line.erase(site,line.length());
      site = line.find('\n');
      if (site >= 0) line.erase(site,line.length());
      site = line.find(' ');
      while (site >= 0) {
         line.erase(site,1);
         site = line.find(' ');
      }
      for (ii=0;ii<line.length();ii++) {
          if (isxdigit(line.at(ii)) == 0)
          {
              return true;
          }
      }
      if (line.length() > 0) {
         valued = 0;
         sscanf(line.c_str(),"%lf",&pixel);
         valued = floor(pixel);
         if (valued > max_din) valued = max_din;
         if (valued < min_din) valued = min_din;
         //result = int642int96(valued);
         //int962hex(result,hexdata,DIN_WIDTH);
         //for (kk=(DIN_WIDTH+3)/4-1;kk>=0;kk--) fprintf(fhw,"%X",hexdata[kk]);
         //fprintf(fhw,"\n");
         kk = idx/(VINHEIGHT*VINWIDTH);
         jj = (idx%(VINHEIGHT*VINWIDTH))/VINWIDTH;
         ii = idx%VINWIDTH;
         mm = (jj+TOP_PAD);
         nn = (ii+LEFT_PAD);
         dmem[kk][mm][nn] = valued;
         idx += 1;
      }
   }
   TOTAL_INPUT += NUM_PLANE*VINWIDTH*VINHEIGHT;
   fclose(fhr);

   delete [] hexdata;

   return false;
}
bool scaler::gen_stimulus_rand(FILE *fhw, int numf)
{
   sint64  temp;
   int96   result;
   int     *hexdata;
   int     cc,ii,jj,kk,mm,nn;
   int     fwidth, fheight;
   unsigned short int *frm;

   //FILE* fh_C1;
   //FILE* fh_C2;
   //FILE* fh_C3;

   //fh_C1 = fopen(("input_file_blue_" + to_string(VINWIDTH) + "x" + to_string(VINHEIGHT) + ".txt").c_str(), "wb");
   //if (fh_C1 == NULL) return 1;
   //fh_C2 = fopen(("input_file_green_" + to_string(VINWIDTH) + "x" + to_string(VINHEIGHT) + ".txt").c_str(), "wb");
   //if (fh_C2 == NULL) return 1;
   //fh_C3 = fopen(("input_file_red_" + to_string(VINWIDTH) + "x" + to_string(VINHEIGHT) + ".txt").c_str(), "wb");
   //if (fh_C3 == NULL) return 1;

   fwidth = CUR_FRMWIDTH[numf]  + 1;
   fheight= CUR_FRMHEIGHT[numf] + 1;
   frm    = new unsigned short int [fwidth*fheight];
   hexdata = new int[(DIN_WIDTH+3)/4+1];
   for (cc=0;cc<NUM_PLANE;cc++) {
      for (ii=0;ii<VINHEIGHT;ii++) {
         for (jj=0;jj<VINWIDTH;jj++) {
            if (DEBUG == 0) {
               if (DIN_TYPE)       temp = (sint64)(((double)(rand())/RAND_MAX-0.5)*max_din*2);
               else                temp = (sint64)(((double)(rand())/RAND_MAX)*max_din);
               if (temp > max_din) temp = max_din;
               if (temp < min_din) temp = min_din;
            } else if (DIN_TYPE == 1){
               temp = -(jj+1)%max_din;
            } else {
               temp = (cc+jj+1+ii)%max_din;
            }
            mm = (ii+TOP_PAD);
            nn = (jj+LEFT_PAD);
            dmem[cc][mm][nn] = temp;
            //if((ii<=CUR_FRMHEIGHT[numf]) && (jj<=CUR_FRMWIDTH[numf])) {
            //   result = int642int96(temp);
            //   int962hex(result,hexdata,DIN_WIDTH);
            //   for (kk=(DIN_WIDTH+3)/4-1;kk>=0;kk--) fprintf(fhw,"%X",hexdata[kk]);
            //   fprintf(fhw,"\n");
            //}

            /*if ((ii <= CUR_FRMHEIGHT[numf]) && (jj <= CUR_FRMWIDTH[numf]))
            {
                result = int642int96(temp);
                int962hex(result, hexdata, DIN_WIDTH);
                if (cc == 0)
                {
                    for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C1, "%X", hexdata[kk]);
                    fprintf(fh_C1, "\n");
                }
                else if (cc == 1)
                {
                    for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C2, "%X", hexdata[kk]);
                    fprintf(fh_C2, "\n");
                }
                else if (cc == 2)
                {
                    for (kk = (DIN_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C3, "%X", hexdata[kk]);
                    fprintf(fh_C3, "\n");
                }
            }*/
         }
      }
   }
   for (cc=0;cc<NUM_PLANE;cc++) {
      for (ii=0;ii<VINHEIGHT;ii++) {
         for (jj=0;jj<VINWIDTH;jj++) {
            mm = (ii+TOP_PAD);
            nn = (jj+LEFT_PAD);
            temp = dmem[cc][mm][nn];
            if((jj<fwidth) && (ii<fheight)) {
               if(YCBCR422==1 && cc>0 && (jj%2)==0) {
                  frm[ii*fwidth/2+jj/2] = (unsigned short int)temp;
               } else {
                  frm[ii*fwidth+jj] = (unsigned short int)temp;
               }
            }
         }
      }
      if(YCBCR422==1 && cc>0) {
         //fwrite(frm,2,fwidth*fheight/2,fhw);
         TOTAL_INPUT += fwidth*fheight/2;
      } else {
         //fwrite(frm,2,fwidth*fheight,fhw); 
         TOTAL_INPUT += fwidth*fheight;
      }
   }

   //fclose(fh_C1);
   //fclose(fh_C2);
   //fclose(fh_C3);

   delete [] hexdata;
   delete [] frm;
   return false;
}
bool scaler::gen_stimulus_from_yuv(FILE * fhr, FILE * fhw)
{
   int hh,ii,jj,kk,num,pix;
   unsigned char *frm;
   int  *hexdata;
   bool rst;
   unsigned short int  *frmin;
   rst = true;
   hexdata  = new int[(DIN_WIDTH+3)/4+1];
   frm      = new unsigned char[2*VINWIDTH*VINHEIGHT];
   frmin    = new unsigned short int[VINWIDTH*VINHEIGHT];
   if(YCBCR422==1) {
      num = fread(frm ,1,2*VINWIDTH*VINHEIGHT,fhr);
      if(num<VINWIDTH*VINHEIGHT) {
         rst = false;
      }
      // Y color
      for(ii=0;ii<VINHEIGHT;ii++) {
         for(jj=0;jj<VINWIDTH;jj++) {
            pix = (int)frm[ii*VINWIDTH+jj];
            dmem[0][ii+TOP_PAD][jj+LEFT_PAD] = pix;
            frmin[ii*VINWIDTH+jj] = (unsigned short int)pix;
         }
      }
      //fwrite(frmin,2,VINWIDTH*VINHEIGHT,fhw);
      // Cb color
      for(ii=0;ii<VINHEIGHT;ii++) {
         for(jj=0;jj<VINWIDTH;jj++) {
            pix = (int)frm[VINHEIGHT*VINWIDTH + ii*VINWIDTH/2+jj/2];
            dmem[1][ii+TOP_PAD][jj+LEFT_PAD] = pix;
            frmin[ii*VINWIDTH/2+jj/2] = (unsigned short int)pix;
         }
      }
      //fwrite(frmin,2,VINWIDTH*VINHEIGHT/2,fhw);
      // Cr color
      for(ii=0;ii<VINHEIGHT;ii++) {
         for(jj=0;jj<VINWIDTH;jj++) {
            pix = (int)frm[3*VINHEIGHT*VINWIDTH/2 + ii*VINWIDTH/2+jj/2];
            dmem[2][ii+TOP_PAD][jj+LEFT_PAD] = pix;
            frmin[ii*VINWIDTH/2+jj/2] = (unsigned short int)pix;
         }
      }
      //fwrite(frmin,2,VINWIDTH*VINHEIGHT/2,fhw);
   } else {
      for(hh=0;hh<NUM_PLANE;hh++) {
         num = fread(frm ,1,VINWIDTH*VINHEIGHT,fhr);
         if(num<VINWIDTH*VINHEIGHT) {
            rst = false; break;
         }
         for(ii=0;ii<VINHEIGHT;ii++) {
            for(jj=0;jj<VINWIDTH;jj++) {
               pix = (int)frm[ii*VINWIDTH+jj];
               dmem[hh][ii+TOP_PAD][jj+LEFT_PAD] = pix;
               frmin[ii*VINWIDTH+jj] = (unsigned short int)pix;
            }
         }
         //fwrite(frmin,2,VINWIDTH*VINHEIGHT,fhw);
      }
   }
   delete [] frm;
   delete [] frmin;
   delete [] hexdata;
   return rst;
}
bool scaler::output_golden_yuv(FILE * fhw)
{
   int hh,ii,jj,kk;
   unsigned char *frm;
   frm = new unsigned char[VOUTWIDTH*VOUTHEIGHT];
   if(YCBCR422==1) {
      // Y
      for(ii=0;ii<VOUTHEIGHT;ii++) {
         for(jj=0;jj<VOUTWIDTH;jj++) {
            frm[ii*VOUTWIDTH+jj] = (unsigned char)gmem[0][ii][jj];
         }
      }
      //fwrite(frm ,1,VOUTWIDTH*VOUTHEIGHT,fhw);
      // Cb
      for(ii=0;ii<VOUTHEIGHT;ii++) {
         for(jj=0;jj<VOUTWIDTH/2;jj++) {
            frm[ii*VOUTWIDTH/2+jj] = (unsigned char)gmem[1][ii][jj*2];
         }
      }
      //fwrite(frm ,1,VOUTWIDTH*VOUTHEIGHT/2,fhw);
      // Cr
      for(ii=0;ii<VOUTHEIGHT;ii++) {
         for(jj=0;jj<VOUTWIDTH/2;jj++) {
            frm[ii*VOUTWIDTH/2+jj] = (unsigned char)gmem[2][ii][jj*2+1];
         }
      }
      //fwrite(frm ,1,VOUTWIDTH*VOUTHEIGHT/2,fhw);
   } else {
      for(hh=0;hh<NUM_PLANE;hh++) {
         for(ii=0;ii<VOUTHEIGHT;ii++) {
            for(jj=0;jj<VOUTWIDTH;jj++) {
               frm[ii*VOUTWIDTH+jj] = (unsigned char)gmem[hh][ii][jj];
            }
         }
         //fwrite(frm ,1,VOUTWIDTH*VOUTHEIGHT,fhw);
      }
   }
   delete [] frm;
   return 0;
}
int scaler::luma_detector(int a, int b, int c)
{  // a  b  c
   int ab, cb, ac;
   int r1,r2,r3;
   int s_bc,s_ba,s_bb;
   int s1,s2,s3,res;

   ac = abs(a - c);
   cb = abs(c - b);
   ab = abs(a - b);

   r1 = (a<c) ? 1 : 0;
   r2 = (c<b) ? 1 : 0;
   r3 = (a<b) ? 1 : 0;

   s_bc = ((r3==0) && (r2==0) && (r1==0)) || ((r3==1) && (r2==1) && (r1==1)); // a>=c>=b || b > c > a
   s_ba = ((r3==0) && (r2==0) && (r1==1)) || ((r3==1) && (r2==1) && (r1==0)); // c> a>=b || b > a >=c
   s_bb = ((r3==0) && (r2==1) && (r1==0)) || ((r3==1) && (r2==0) && (r1==1)); // a>=b> c || c >=b > a

   s1   = (s_ba==1) || (s_bb==1 && (ab <= (ac/4)));    // near a, ab<=ac/4
   s2   = (s_bb==1) && (ab > (ac/4)) && (cb > (ac/4)); // near b, ab>ac/4 && bc>ac/4
   s3   = (s_bc==1) || (s_bb==1 && (cb <= (ac/4)));    // near c, bc<=ac/4
   if(s1==1)      res = 1; // near a
   else if(s2==1) res = 2; // near b
   else if(s3==1) res = 4; // near c
   else           res = 1;

   return res;
}
bool scaler::pad_frame(int numf)
{
   int value;
   int ii,jj,kk;
   int fwidth,fheight;
   int mm,nn;
   fwidth = CUR_FRMWIDTH[numf]  + 1 +LEFT_PAD+RIGHT_PAD;
   fheight= CUR_FRMHEIGHT[numf] + 1 +TOP_PAD+BOTTOM_PAD;
   for(ii=0;ii<NUM_PLANE;ii++) {
      //YCBCR422 
      if(YCBCR422==1 && ii>0) {
         for (jj=0;jj<fheight;jj++) {
            for (kk=0;kk<fwidth;kk++) {
               nn = (kk-LEFT_PAD);
               // copy to construct 444 before averaging
               if((nn%2)==1) {
                  dmem[ii][jj][kk] = dmem[ii][jj][kk-1];
               }
            }
         }
      }
      if(PAD_TYPE==0) {// pad value
         for(kk=0;kk<fwidth;kk++) {
            for(jj=0;jj<TOP_PAD;jj++) {
               dmem[ii][(               jj)][kk] = PAD_VALUE;
            }
            for(jj=0;jj<BOTTOM_PAD;jj++) {
               dmem[ii][(fheight-1-jj)][kk] = PAD_VALUE;
            }
         }
         for(jj=0;jj<fheight;jj++) {
            for(kk=0;kk<LEFT_PAD;kk++) {
               dmem[ii][jj][(              kk)] = PAD_VALUE;
            }
            for(kk=0;kk<RIGHT_PAD;kk++) {
               dmem[ii][jj][(fwidth-1-kk)] = PAD_VALUE;
            }
         }
      } else if(PAD_TYPE==1) {// pad copy
         for(kk=0;kk<fwidth;kk++) {
            for(jj=0;jj<TOP_PAD;jj++) {
               dmem[ii][(               jj)][kk] = dmem[ii][(                  TOP_PAD)][kk];
            }
            for(jj=0;jj<BOTTOM_PAD;jj++) {
               dmem[ii][(fheight-1-jj)][kk] = dmem[ii][(fheight-1-BOTTOM_PAD)][kk];
            }
         }
         for(jj=0;jj<fheight;jj++) {
            for(kk=0;kk<LEFT_PAD;kk++) {
               dmem[ii][jj][(              kk)] = dmem[ii][jj][(               LEFT_PAD)];
            }
            for(kk=0;kk<RIGHT_PAD;kk++) {
               dmem[ii][jj][(fwidth-1-kk)] = dmem[ii][jj][(fwidth-1-RIGHT_PAD)];
            }
         }
      } else if(PAD_TYPE==2) {// pad mirror
         for(kk=0;kk<fwidth;kk++) {
            for(jj=0;jj<TOP_PAD;jj++) {
               dmem[ii][(               jj)][kk] = dmem[ii][(              2*TOP_PAD-1-jj)][kk];
            }
            for(jj=0;jj<BOTTOM_PAD;jj++) {
               dmem[ii][(fheight-1-jj)][kk] = dmem[ii][(fheight-2*BOTTOM_PAD+jj)][kk];
            }
         }
         for(jj=0;jj<fheight;jj++) {
            for(kk=0;kk<LEFT_PAD;kk++) {
               dmem[ii][jj][(              kk)] = dmem[ii][jj][(           2*LEFT_PAD-1-kk)];
            }
            for(kk=0;kk<RIGHT_PAD;kk++) {
               dmem[ii][jj][(fwidth-1-kk)] = dmem[ii][jj][(fwidth-2*RIGHT_PAD+kk)];
            }
         }
      }
   }
   return false;
}

int scaler::run_scaling()
{
   bool status = false;
   FILE *fh_sti,*fh_gld;
   FILE *fh_yin,*fh_yout;
   int  numc;
   int  hexnum = (DOUT_WIDTH+3)/4;
   int  ii,jj,kk;
   int  numf;

   fh_sti = fopen("stimulus.dat","wb");
   if (fh_sti == NULL) return 1;
   fh_gld = fopen("golden.dat","wb");
   if (fh_gld == NULL) return 1;

   numf = 0;
   if(LOADSTIMULUS==1) {
      if(LOADYUVFILE==false) {
         if(LOADBMPFILE) gen_stimulus_from_bmp(fh_sti);
         else            gen_stimulus_from_file(fh_sti);
         pad_frame(0);
         status = filter_frame(0);
         if(LOADBMPFILE) {
            gen_input_bmp();
            gen_output_bmp();
         }
         output_golden(fh_gld,0);
         if(status) return 3;
      } else {
         fh_yin = fopen(STIMULUSFILE,"rb");
         if (fh_yin == NULL)
         {
             printf("Error opening %s file.\n", STIMULUSFILE);
             return 1;
         }
         fh_yout= fopen("golden.yuv","wb");
         if (fh_yout == NULL)
         {
             printf("Error opening golden.yuv file.\n");
             return 1;
         }
         while(!feof(fh_yin)) {
            if(gen_stimulus_from_yuv(fh_yin,fh_sti)) {
               pad_frame(0);
               filter_frame(0);
               output_golden(fh_gld,0);
               output_golden_yuv(fh_yout);
               NUM_FRAME++;
            }
         }
         fclose(fh_yin);
         fclose(fh_yout);
      }
   } else {
      for(ii=0;ii<NUM_FRAME;ii++) {
         gen_stimulus_rand(fh_sti,ii);
         pad_frame(ii);
         filter_frame(ii); // filtering
         output_golden(fh_gld,ii);
      }
   }
   fclose(fh_sti);
   fclose(fh_gld);

   int result = std::remove("stimulus.dat");

   if (result != 0) {
       std::perror("Error deleting file");
       return 1;
   }

   result = std::remove("golden.dat");

   if (result != 0) {
       std::perror("Error deleting file");
       return 1;
   }

   return 0;
}
bool scaler::filter_frame(int numf)
{
   return filter_separable(numf);
}
sint64 scaler::filter_vertical(int cc, int hh, int ww, int numc)
{
   sint64 result;
   sint64 tmp;
   int ii,jj,numc_t,cs,oft;
   result = 0;
      cs = 0; //default coefficient set
      if(KERNEL==4 && ADAPTIVE==1) {// for edge adaptive scaling algorithm
         tmp = 0;
         oft = (VFTAPS-4)/2;
         if(YCBCR422==1 || YCBCR444==1) { // only depend on Luma
            for(ii=oft;ii<VFTAPS-oft;ii++) {
               if(ii==VFTAPS/2 && (VFTAPS%2)==1) tmp = tmp; // do not use middle one for ODD TAPS
               else if(ii<VFTAPS/2)              tmp = tmp + (sint64)dmem[0][hh+ii][ww];
               else                              tmp = tmp - (sint64)dmem[0][hh+ii][ww];
            }
            if(tmp > 2*EDGE_TH || tmp < -2*EDGE_TH) cs = 1;
            else                                    cs = 0;
            if(YCBCR422==1 && PARALLEL==1 && ((ww-LEFT_PAD)%2)==1) // odd and even pixels shares the same cs value for horizontal processing
               cs = PRE_EVEN_CS;
         } else {
            for(jj=0;jj<NUM_PLANE;jj++) {
               for(ii=oft;ii<VFTAPS-oft;ii++) {
                  if(ii==VFTAPS/2 && (VFTAPS%2)==1) tmp = tmp; // do not use middle one for ODD TAPS
                  else if(ii<VFTAPS/2)              tmp = tmp + (sint64)dmem[jj][hh+ii][ww];
                  else                              tmp = tmp - (sint64)dmem[jj][hh+ii][ww];
               }
            }
            if(tmp > NUM_PLANE*2*EDGE_TH || tmp < -NUM_PLANE*2*EDGE_TH) cs = 1;
            else                                    cs = 0;
         }
         // force to disable for upscaling
         if(V_UPSCALING==1) cs = 0;
      }
      if(KERNEL==4 && ADAPTIVE==0 && CBANKS==2) cs = (V_UPSCALING==1) ? 0 : 1;
      PRE_EVEN_CS = cs;
      if(KERNEL==5) cs = CSETS_CS; // only for custom kernel coeffs
      for(ii=0;ii<VFTAPS;ii++) {
         tmp    = (sint64)dmem[cc][hh+ii][ww] * (sint64)vcmem[cs][numc][ii];
         result = result + tmp;
      }
   
   return result;
}
sint64 scaler::filter_horizontal(int cc, int hh, int ww, int numc, int pre_cs)
{
   sint64 result;
   sint64 tmp;
   int ii,jj,numc_t,cs,oft;
   int *tmem;
   tmem = hmem[cc][hh];
   result = 0;
      cs = 0; //default coefficient set
      if(KERNEL==4 && ADAPTIVE==1) {// for edge adaptive scaling algorithm
         tmp = 0;
         oft = (HFTAPS-4)/2;
         if(YCBCR422==1 || YCBCR444==1) { // only depend on Luma
            for(ii=oft;ii<HFTAPS-oft;ii++) {
               if(ii==HFTAPS/2 && (HFTAPS%2)==1) tmp = tmp; // do not use middle one for ODD TAPS
               else if(ii<HFTAPS/2)              tmp = tmp + hmem[0][hh][ww+ii];
               else                              tmp = tmp - hmem[0][hh][ww+ii];
            }
            if(tmp > 2*EDGE_TH || tmp < -2*EDGE_TH) cs = 1;
            else                                    cs = 0;
            if(pre_cs != -1) cs = pre_cs;
         } else {
            for(jj=0;jj<NUM_PLANE;jj++) {
               for(ii=oft;ii<HFTAPS-oft;ii++) {
                  if(ii==HFTAPS/2 && (HFTAPS%2)==1) tmp = tmp; // do not use middle one for ODD TAPS
                  else if(ii<HFTAPS/2)              tmp = tmp + hmem[jj][hh][ww+ii];
                  else                              tmp = tmp - hmem[jj][hh][ww+ii];
               }
            }
            if(tmp > NUM_PLANE*2*EDGE_TH || tmp < -NUM_PLANE*2*EDGE_TH) cs = 1;
            else                                    cs = 0;
         }
         // force to disable for upscaling
         if(H_UPSCALING==1) cs = 0;
      }
      if(KERNEL==4 && ADAPTIVE==0 && CBANKS==2) cs = (H_UPSCALING==1) ? 0 : 1;
      PRE_EVEN_CS = cs;
      if(KERNEL==5) cs = CSETS_CS; // only for custom kernel coeffs
      for(ii=0;ii<HFTAPS;ii++) {
         tmp    = tmem[ww+ii] * (sint64)hcmem[cs][numc][ii];
         result = result + tmp;
      }
   
   return result;
}
bool scaler::filter_separable(int numf)
{
   int  ii,jj,kk,hh,ww,nn,total,ldt;
   int     *tmem;
   int96   result;
   sint64  value,tmp;
   sint64  fcrdy,fcrdx; // floating cordinate of x and y
   sint64  dcrdy;   // y decimal cordinate
   sint64  dcrdx;   // x decimal cordinate
   sint64  yphase; //
   sint64  xphase,xphase_r; //
   sint64  SCALE_WIDTH,OUT_WIDTH,OUT_HEIGHT;
   sint64  VSFACTOR,HSFACTOR;
   //tmem    = new sint64[IMAGE_WIDTH+10];

   int* hexdata;
   hexdata = new int[(DOUT_WIDTH + 3) / 4 + 1];
   //FILE* fhw;
   //fhw = fopen("vertical_filter_output.txt", "wb");
   //if (fhw == NULL) {
   //    return true;
   //}

   // update params for the numf frame
   SCALE_WIDTH  = CUR_FRMWIDTH[numf]+1 + HFTAPS-1;
   OUT_WIDTH    = CUR_OUTWIDTH[numf]+1;
   OUT_HEIGHT   = CUR_OUTHEIGHT[numf]+1;
   VSFACTOR     = floor(((double)VFBVALUE*(CUR_FRMHEIGHT[numf]+1))/OUT_HEIGHT);
   HSFACTOR     = floor(((double)HFBVALUE*(CUR_FRMWIDTH[numf]+1))/OUT_WIDTH);
   //HSFACTOR = HSFACTOR * 2; - Hardcode
   V_UPSCALING  = (CUR_OUTHEIGHT[numf] >=CUR_FRMHEIGHT[numf]) ? 1 : 0;
   H_UPSCALING  = ( CUR_OUTWIDTH[numf] >= CUR_FRMWIDTH[numf]) ? 1 : 0;
   // filtering process
   for(ii=0;ii<NUM_PLANE;ii++) {
      //fcrdy = VSFACTOR/2 + VFBVALUE/2; //consistent with matlab
      fcrdy = 0;
      for(hh=0;hh<OUT_HEIGHT;hh++) {
         dcrdy   = (fcrdy/VFBVALUE);
         yphase = (fcrdy%VFBVALUE)/(VFBVALUE/VFPHASES);
         if(dcrdy>VINHEIGHT) dcrdy = VINHEIGHT;
         for(ww=0;ww<SCALE_WIDTH;ww++) { //vertical filtering
            value = filter_vertical(ii,dcrdy,ww,yphase);
            if(KERNEL>1) value = value + (value>>COEFF_POINTS); //compensation for coeffs sum value
            result = bitproc(int642int96(value),DIN_TYPE,COEFF_TYPE,DIN_TYPE,DIN_POINTS,COEFF_POINTS,MOUT_POINTS,MOUT_WIDTH,YFULL_WIDTH,LSB_METHOD,MSB_METHOD);
            hmem[ii][hh][ww] = int962int64(result,MOUT_WIDTH);

            //result = int642int96(value);
            //int962hex(result,hexdata,DOUT_WIDTH);
            //for (kk=(DOUT_WIDTH+3)/4-1;kk>=0;kk--) fprintf(fhw,"%X",hexdata[kk]);
            //fprintf(fhw,"\n");
         }
         fcrdy += VSFACTOR;
      }
   }
   //fclose(fhw);

#if 0
   FILE* fhw_1;
   FILE* fhw_2;
   FILE* fhw_3;
   fhw_1 = fopen("Y_vertical_output.txt", "wb");
   if (fhw_1 == NULL) {
       return true;
   }
   fhw_2 = fopen("Cb_vertical_output.txt", "wb");
   if (fhw_2 == NULL) {
       return true;
   }
   fhw_3 = fopen("Cr_vertical_output.txt", "wb");
   if (fhw_3 == NULL) {
       return true;
   }
   int  fwidth, fheight;
   fwidth = CUR_OUTWIDTH[numf] + 1;
   fheight = CUR_OUTHEIGHT[numf] + 1;
   if (YCBCR422 == 1) {
       // Y
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < SCALE_WIDTH; ww++) {
               if ((ww < fwidth) && (jj < fheight)) {
                   value = hmem[0][jj][ww];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_1, "%X", hexdata[kk]);
                   fprintf(fhw_1, "\n");
               }
           }
       }

       // Cb
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < SCALE_WIDTH / 2; ww++) {
               if ((ww < fwidth / 2) && (jj < fheight)) {
                   value = hmem[1][jj][ww * 2];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_2, "%X", hexdata[kk]);
                   fprintf(fhw_2, "\n");
               }
           }
       }

       // Cr
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < SCALE_WIDTH / 2; ww++) {
               if ((ww < fwidth / 2) && (jj < fheight)) {
                   value = hmem[2][jj][ww * 2 + 1];
                   //value = hmem[2][jj][ww * 2];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_3, "%X", hexdata[kk]);
                   fprintf(fhw_3, "\n");
               }
           }
       }
   }
   fclose(fhw_1);
   fclose(fhw_2);
   fclose(fhw_3);
#endif
   //Vertical filter done

   //fhw = fopen("horizontal_filter_output.txt", "wb");
   //if (fhw == NULL) {
   //    return true;
   //}
   for(ii=0;ii<NUM_PLANE;ii++) {
      for(hh=0;hh<OUT_HEIGHT;hh++) {
         //======= remove PAD_TYPE_0
         tmem=hmem[ii][hh];
         if(KERNEL==4 && ADAPTIVE==1 && YCBCR422==1) {// re-pad the edge pixel
               for(jj=SCALE_WIDTH-RIGHT_PAD-1;jj<SCALE_WIDTH;jj++) {
                  tmem[jj] = tmem[SCALE_WIDTH-RIGHT_PAD-1];
               }
               for(jj=0;jj<LEFT_PAD+1;jj++) {
                  tmem[jj] = tmem[LEFT_PAD];
               }
         }
         if(YCBCR422==1 && ii>0) {// to re-construct 444 format
            for(kk=0;kk<(SCALE_WIDTH-(HFTAPS-1)-1);kk++) { //except the last pixel
               if((kk%2)==1) {
                  nn = kk+LEFT_PAD;
                  if(RESAMPLE==2) {
                     ldt = luma_detector(hmem[0][hh][nn+1],hmem[0][hh][nn], hmem[0][hh][nn-1]);
                     if(ldt==1)      tmem[nn] = tmem[nn+1];                       // right
                     else if(ldt==2) tmem[nn] = (tmem[nn-1] + tmem[nn+1] + 1)>>1; //middle
                     else            tmem[nn] = tmem[nn-1];                       // left
                  } else if(RESAMPLE==1) {
                     tmem[nn] = (tmem[nn-1] + tmem[nn+1] + 1)>>1; // consistent with verilog >>
                  } else {
                     tmem[nn] = tmem[nn-1];
                  }
               }
            }
         }
         //fcrdx = HSFACTOR/2 + HFBVALUE/2; //consistent with matlab
         fcrdx = 0;
         for(ww=0;ww<OUT_WIDTH;ww++) { //horizontal filtering
            dcrdx  = (fcrdx/HFBVALUE);
            xphase = (fcrdx%HFBVALUE)/(HFBVALUE/HFPHASES);
            if(dcrdx>VINWIDTH) dcrdx = VINWIDTH;
            if(YCBCR422==1 && PARALLEL==1 && SHARE_CMEM==0 && (ww%2)==1 && ii>0) // only for YCbCr422 parallel scaling
               value = filter_horizontal(ii,hh,dcrdx,xphase_r,PRE_EVEN_CS);
            else
               value = filter_horizontal(ii,hh,dcrdx,xphase,-1);
            if(KERNEL>1) value = value + (value>>COEFF_POINTS); //compensation for coeffs sum value
            result= bitproc(int642int96(value),DIN_TYPE,COEFF_TYPE,DOUT_TYPE,MOUT_POINTS,COEFF_POINTS,DOUT_POINTS,DOUT_WIDTH,XFULL_WIDTH,LSB_METHOD,MSB_METHOD);
            gmem[ii][hh][ww] = int962int64(result,DOUT_WIDTH);
            
            //result = int642int96(value);
            //int962hex(result,hexdata,DOUT_WIDTH);
            //for (kk=(DOUT_WIDTH+3)/4-1;kk>=0;kk--) fprintf(fhw,"%X",hexdata[kk]);
            //fprintf(fhw,"\n");

            fcrdx += HSFACTOR;
            xphase_r = xphase;
         }
      }
   }
   //delete [] tmem;
   //fclose(fhw);
#if 0
   fhw_1 = fopen("Y_horizontal_output.txt", "wb");
   if (fhw_1 == NULL) {
       return true;
   }
   fhw_2 = fopen("Cb_horizontal_output.txt", "wb");
   if (fhw_2 == NULL) {
       return true;
   }
   fhw_3 = fopen("Cr_horizontal_output.txt", "wb");
   if (fhw_3 == NULL) {
       return true;
   }
  
   fwidth = CUR_OUTWIDTH[numf] + 1;
   fheight = CUR_OUTHEIGHT[numf] + 1;
   if (YCBCR422 == 1) {
       // Y
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < OUT_WIDTH; ww++) {
               if ((ww < fwidth) && (jj < fheight)) {
                   value = gmem[0][jj][ww];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_1, "%X", hexdata[kk]);
                   fprintf(fhw_1, "\n");
               }
           }
       }

       // Cb
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < OUT_WIDTH / 2; ww++) {
               if ((ww < fwidth / 2) && (jj < fheight)) {
                   value = gmem[1][jj][ww * 2];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_2, "%X", hexdata[kk]);
                   fprintf(fhw_2, "\n");
               }
           }
       }

       // Cr
       for (jj = 0; jj < FOUT_HEIGHT; jj++) {
           for (ww = 0; ww < OUT_WIDTH / 2; ww++) {
               if ((ww < fwidth / 2) && (jj < fheight)) {
                   //value = gmem[2][jj][ww * 2 + 1];
                   value = gmem[2][jj][ww * 2];
                   result = int642int96(value);
                   int962hex(result, hexdata, DOUT_WIDTH);
                   for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fhw_3, "%X", hexdata[kk]);
                   fprintf(fhw_3, "\n");
               }
           }
       }
   }
   fclose(fhw_1);
   fclose(fhw_2);
   fclose(fhw_3);

#endif
   return 0;
}
bool scaler::gen_stimulus()
{
   return false;
}

bool scaler::gen_input_bmp()
{
    FILE* fh_bmp;
    int ii, jj, kk, pp;
    int IMAGEH, IMAGEW, PLANES, NN, CLINE;
    unsigned char     pixel;
    int96  v96;
    sint64 v64;
    fh_bmp = fopen("testbench/input.bmp", "wb");
    if (fh_bmp == NULL) return true;
    NN = (bfinfo.BDO - 54) / 4;

    bfinfo.BDS = bfinfo.BDS - (bfinfo.IMW * bfinfo.IMH * bfinfo.BPP / 8);
    bfinfo.FS = bfinfo.FS - (bfinfo.IMW * bfinfo.IMH * bfinfo.BPP / 8);
    bfinfo.IMW = VINWIDTH;
    bfinfo.IMH = VINHEIGHT;
    bfinfo.BDS = (((bfinfo.IMW * bfinfo.BPP / 8) + 3) / 4) * 4 * bfinfo.IMH;
    bfinfo.FS = bfinfo.BDO + bfinfo.BDS;
    fwrite(&bfinfo.ID, 1, 2, fh_bmp);
    fwrite(&bfinfo.FS, 1, 4, fh_bmp);
    fwrite(&bfinfo.RS0, 1, 4, fh_bmp);
    fwrite(&bfinfo.BDO, 1, 4, fh_bmp);
    fwrite(&bfinfo.BHS, 1, 4, fh_bmp);
    fwrite(&bfinfo.IMW, 1, 4, fh_bmp);
    fwrite(&bfinfo.IMH, 1, 4, fh_bmp);
    fwrite(&bfinfo.PS, 1, 2, fh_bmp);
    fwrite(&bfinfo.BPP, 1, 2, fh_bmp);
    fwrite(&bfinfo.CPN, 1, 4, fh_bmp);
    fwrite(&bfinfo.BDS, 1, 4, fh_bmp);
    fwrite(&bfinfo.HRL, 1, 4, fh_bmp);
    fwrite(&bfinfo.VRL, 1, 4, fh_bmp);
    fwrite(&bfinfo.CS, 1, 4, fh_bmp);
    fwrite(&bfinfo.ICS, 1, 4, fh_bmp);
    fwrite(bfinfo.CP, 4, NN, fh_bmp);

    IMAGEW = bfinfo.IMW;
    IMAGEH = bfinfo.IMH;
    PLANES = bfinfo.BPP / 8;
    CLINE = ((IMAGEW * PLANES + 3) / 4) * 4;
    for (jj = 0; jj < IMAGEH; jj++) {
        for (pp = 0; pp < CLINE; pp++) {
            ii = pp % PLANES;
            kk = pp / PLANES;
            if ((jj < IMAGE_HEIGHT) && (kk < IMAGE_WIDTH)) {
                v64 = dmem[ii][IMAGE_HEIGHT - 1 - jj][kk];
            }
            else {
                v64 = 0;
            }
            if (v64 > 255) {
                v64 = 255;
            }
            pixel = v64;
            fwrite(&pixel, 1, 1, fh_bmp);
        }
    }

    fclose(fh_bmp);
    //delete [] bfinfo.CP;
    return false;
}

bool scaler::gen_output_bmp()
{
   FILE *fh_bmp;
   int ii,jj,kk,pp;
   int IMAGEH,IMAGEW,PLANES,NN,CLINE;
   unsigned char     pixel;
   int96  v96;
   sint64 v64;
   fh_bmp = fopen("testbench/golden.bmp","wb");
   if (fh_bmp == NULL) return true;
   NN = (bfinfo.BDO-54)/4;

   bfinfo.BDS = bfinfo.BDS - (bfinfo.IMW*bfinfo.IMH*bfinfo.BPP/8);
   bfinfo.FS  = bfinfo.FS  - (bfinfo.IMW*bfinfo.IMH*bfinfo.BPP/8);
   bfinfo.IMW = VOUTWIDTH;
   bfinfo.IMH = VOUTHEIGHT;
   bfinfo.BDS = (((bfinfo.IMW*bfinfo.BPP/8)+3)/4)*4*bfinfo.IMH;
   bfinfo.FS  = bfinfo.BDO + bfinfo.BDS;
   fwrite(&bfinfo.ID, 1,2,fh_bmp);
   fwrite(&bfinfo.FS, 1,4,fh_bmp);
   fwrite(&bfinfo.RS0,1,4,fh_bmp);
   fwrite(&bfinfo.BDO,1,4,fh_bmp);
   fwrite(&bfinfo.BHS,1,4,fh_bmp);
   fwrite(&bfinfo.IMW,1,4,fh_bmp);
   fwrite(&bfinfo.IMH,1,4,fh_bmp);
   fwrite(&bfinfo.PS, 1,2,fh_bmp);
   fwrite(&bfinfo.BPP,1,2,fh_bmp);
   fwrite(&bfinfo.CPN,1,4,fh_bmp);
   fwrite(&bfinfo.BDS,1,4,fh_bmp);
   fwrite(&bfinfo.HRL,1,4,fh_bmp);
   fwrite(&bfinfo.VRL,1,4,fh_bmp);
   fwrite(&bfinfo.CS, 1,4,fh_bmp);
   fwrite(&bfinfo.ICS,1,4,fh_bmp);
   fwrite(bfinfo.CP,4,NN,fh_bmp);

   IMAGEW = bfinfo.IMW;
   IMAGEH = bfinfo.IMH;
   PLANES = bfinfo.BPP/8;
   CLINE  = ((IMAGEW*PLANES+3)/4)*4;
   for(jj=0;jj<IMAGEH;jj++) {
      for(pp=0;pp<CLINE;pp++) {
         ii = pp%PLANES;
         kk = pp/PLANES;
         if((jj<FOUT_HEIGHT) && (kk<FOUT_WIDTH)) {
            v64 = gmem[ii][FOUT_HEIGHT-1-jj][kk];
         } else {
            v64 = 0;
         }
         if(v64 > 255) {
            v64 = 255;
         }
         pixel = v64;
         fwrite(&pixel,1,1,fh_bmp);
      }
   }

   fclose(fh_bmp);
   delete [] bfinfo.CP;
   return false;
}
bool scaler::output_golden(FILE *fhg, int numg)
{
   int  *hexdata;
   int  value;
   int  hh,ww,ii,jj,kk,mm,nn;
   int  hexnum = (DOUT_WIDTH+3)/4;
   int  fwidth,fheight;
   int  total;
   unsigned short int *frm;

   int96   result;
   FILE* fh_C1;
   FILE* fh_C2;
   FILE* fh_C3;
   if (YCBCR422 == 0 && YCBCR444 == 0) {
       fh_C1 = fopen("testbench/golden_file_blue.txt", "wb");
       if (fh_C1 == NULL) return 1;
       fh_C2 = fopen("testbench/golden_file_green.txt", "wb");
       if (fh_C2 == NULL) return 1;
       fh_C3 = fopen("testbench/golden_file_red.txt", "wb");
       if (fh_C3 == NULL) return 1;
   }
   else {
       fh_C1 = fopen("testbench/golden_file_y.txt", "wb");
       if (fh_C1 == NULL) return 1;
       fh_C2 = fopen("testbench/golden_file_cb.txt", "wb");
       if (fh_C2 == NULL) return 1;
       fh_C3 = fopen("testbench/golden_file_cr.txt", "wb");
       if (fh_C3 == NULL) return 1;
   }

   total   = 0;
   fwidth  = CUR_OUTWIDTH[numg]+1;
   fheight = CUR_OUTHEIGHT[numg]+1;
   hexdata = new int[hexnum];
   frm     = new unsigned short int[fwidth*fheight];
   if(YCBCR422==1) {
      // Y
      for(jj=0;jj<FOUT_HEIGHT;jj++) {
         for(ww=0;ww<FOUT_WIDTH;ww++) {
            if((ww<fwidth) && (jj<fheight)) {
               value = gmem[0][jj][ww];

               result = int642int96(sint64(value));
               int962hex(result, hexdata, DOUT_WIDTH);
               for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C1, "%X", hexdata[kk]);
               fprintf(fh_C1, "\n");

               total += 1;
               frm[jj*fwidth+ww] = (unsigned short int)value;
            }
         }
      }
      //fwrite(frm,2,fwidth*fheight,fhg);
      // Cb
      for(jj=0;jj<FOUT_HEIGHT;jj++) {
         for(ww=0;ww<FOUT_WIDTH/2;ww++) {
            if((ww<fwidth/2) && (jj<fheight)) {
               value = gmem[1][jj][ww*2];

               result = int642int96(sint64(value));
               int962hex(result, hexdata, DOUT_WIDTH);
               for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C2, "%X", hexdata[kk]);
               fprintf(fh_C2, "\n");

               frm[jj*fwidth/2+ww] = (unsigned short int)value;
               if(PARALLEL==0) total += 1;
            }
         }
      }
      //fwrite(frm,2,fwidth*fheight/2,fhg);
      // Cr
      for(jj=0;jj<FOUT_HEIGHT;jj++) {
         for(ww=0;ww<FOUT_WIDTH/2;ww++) {
            if((ww<fwidth/2) && (jj<fheight)) {
               //value = gmem[2][jj][ww*2+1];
               value = gmem[2][jj][ww * 2];

               result = int642int96(sint64(value));
               int962hex(result, hexdata, DOUT_WIDTH);
               for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C3, "%X", hexdata[kk]);
               fprintf(fh_C3, "\n");

               frm[jj*fwidth/2+ww] = (unsigned short int)value;
               if(PARALLEL==0) total += 1;
            }
         }
      }
      //fwrite(frm,2,fwidth*fheight/2,fhg);
   } else {
      for(ii=0;ii<NUM_PLANE;ii++) {
         for(jj=0;jj<FOUT_HEIGHT;jj++) {
            for(ww=0;ww<FOUT_WIDTH;ww++) {
               if((ww<fwidth) && (jj<fheight)) {
                  value = gmem[ii][jj][ww];
                  
                    //result = int642int96(sint64(value));
                    //int962hex(result,hexdata, DOUT_WIDTH);
                    //for (kk=(DOUT_WIDTH +3)/4-1;kk>=0;kk--) fprintf(fhg,"%X",hexdata[kk]);
                    //fprintf(fhg,"\n");
                  result = int642int96(sint64(value));
                  int962hex(result, hexdata, DOUT_WIDTH);
                  if (ii == 0)
                  {
                    for (kk=(DOUT_WIDTH +3)/4-1;kk>=0;kk--) fprintf(fh_C1,"%X",hexdata[kk]);
                    fprintf(fh_C1,"\n");
                  }
                  else if (ii == 1)
                  {
                      for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C2, "%X", hexdata[kk]);
                      fprintf(fh_C2, "\n");
                  }
                  else if (ii == 2)
                  {
                      for (kk = (DOUT_WIDTH + 3) / 4 - 1; kk >= 0; kk--) fprintf(fh_C3, "%X", hexdata[kk]);
                      fprintf(fh_C3, "\n");
                  }
                  frm[jj*fwidth+ww] = (unsigned short int)value;
                  if(PARALLEL==0 || ii==0) total += 1;
               }
            }
         }
         //fwrite(frm,2,fwidth*fheight,fhg);
      }
   }
   TOTAL_OUTPUT += total;

   fclose(fh_C1);
   fclose(fh_C2);
   fclose(fh_C3);
   delete [] hexdata;
   delete [] frm;
   return false;
}

bool scaler::write_mem(char *cmemfile, int **coeffmem, int numc)
{
   return false;
}
bool scaler::regencoeff()
{

   //regenerate the coeffs
   if(KERNEL>1) {
      coeff_gen_v();
      if(SHARE_CMEM==0) {
         coeff_gen_h();
      }
   }

   return false;
}

bool scaler::coeff_gen_v()
{
   string fname;
   FILE *fh;
   int  ii,jj,kk,cc;
   bool *binarray;
   int  *bin;
   int  KOUTWIDTH;

   //generate the rom file
   //fname.append(SCALER_NAME);
   fname.append("coeffv.rom");
   fh = fopen(fname.c_str(),"w");
   if(fh==NULL) return true;
   binarray= new bool[COEFF_WIDTH*VFTAPS];
   bin     = new int[COEFF_WIDTH+4];
   KOUTWIDTH=VFTAPS*COEFF_WIDTH;

   //regenerate the coeffs
   for(cc=0;cc<CBANKS;cc++) {
      for(ii=0;ii<VFPHASES;ii++) {
         for(jj=0;jj<VFTAPS;jj++) {
               dec2bin(vcmem[cc][ii][VFTAPS-1-jj],COEFF_WIDTH,bin);
               for(kk=0;kk<COEFF_WIDTH;kk++) {
                  if(bin[kk] > 0)   binarray[jj*COEFF_WIDTH+kk] = true;
                  else              binarray[jj*COEFF_WIDTH+kk] = false;
               }
         }
         for (jj=KOUTWIDTH-1;jj>=0;jj--) {
            fprintf(fh,"%1d",binarray[jj]);
         }
         fprintf(fh,"\n");
      }
   }

   fclose(fh);

   delete [] bin;
   delete [] binarray;
   return false;
}
bool scaler::coeff_gen_h()
{
   string fname;
   FILE *fh;
   int  ii,jj,kk,cc;
   bool *binarray;
   int  *bin;
   int  KOUTWIDTH;

   //generate the rom file
   //fname.append(SCALER_NAME);
   fname.append("coeffh.rom");
   fh = fopen(fname.c_str(),"w");
   if(fh==NULL) return true;
   binarray= new bool[COEFF_WIDTH*HFTAPS];
   bin     = new int[COEFF_WIDTH+4];
   KOUTWIDTH=HFTAPS*COEFF_WIDTH;

   //regenerate the coeffs
   for(cc=0;cc<CBANKS;cc++) {
      for(ii=0;ii<HFPHASES;ii++) {
         for(jj=0;jj<HFTAPS;jj++) {
               dec2bin(hcmem[cc][ii][HFTAPS-1-jj],COEFF_WIDTH,bin);
               for(kk=0;kk<COEFF_WIDTH;kk++) {
                  if(bin[kk] > 0)   binarray[jj*COEFF_WIDTH+kk] = true;
                  else              binarray[jj*COEFF_WIDTH+kk] = false;
               }
         }
         for (jj=KOUTWIDTH-1;jj>=0;jj--) {
            fprintf(fh,"%1d",binarray[jj]);
         }
         fprintf(fh,"\n");
      }
   }

   fclose(fh);

   delete [] bin;
   delete [] binarray;
   return false;
}

bool scaler::coeff_reload()
{
   return false;
}
int96 scaler::bitproc(int96 din,bool DTYPE,bool CTYPE, bool OTYPE,int DINPTS,int COEPTS,int DOUTPTS,int DOUTW,int FWIDTH,int L_METHOD,int M_METHOD)
{
   int96 dout;
   int96 doutr;
   uint32 a;
   int IDOUTWIDTH;
   bool even_odd;
   uint32 tempp;
   int LSFWIDTH;
   int LSFW;
   int96 temp;
   int L_DIN_POINTS;
   int L_COEFF_POINTS;
   int L_DOUT_POINTS;
   int L_DOUT_WIDTH;
   int L_FULL_WIDTH;
   int L_START_LSB;

   L_DIN_POINTS   = DINPTS;
   L_COEFF_POINTS = COEPTS;
   L_DOUT_POINTS  = DOUTPTS;
   L_DOUT_WIDTH   = DOUTW;
   L_FULL_WIDTH   = FWIDTH;

   if (L_DOUT_POINTS < L_DIN_POINTS+L_COEFF_POINTS) {
      L_START_LSB = L_DIN_POINTS+L_COEFF_POINTS-L_DOUT_POINTS;
   } else {
      L_START_LSB = 0;
   }

   if(L_DOUT_POINTS==(L_DIN_POINTS+L_COEFF_POINTS))
      IDOUTWIDTH = L_DOUT_WIDTH; 
   else if(L_DOUT_POINTS>(L_DIN_POINTS+L_COEFF_POINTS))
      IDOUTWIDTH = L_DOUT_WIDTH-(L_DOUT_POINTS-(L_DIN_POINTS+L_COEFF_POINTS)); 
   else {
      if(L_DOUT_WIDTH<=(L_FULL_WIDTH-L_START_LSB))
         IDOUTWIDTH = L_DOUT_WIDTH; 
      else
         IDOUTWIDTH = L_FULL_WIDTH-L_START_LSB; 
   }
   if (L_DOUT_WIDTH==L_FULL_WIDTH && (L_DOUT_POINTS==(L_DIN_POINTS+L_COEFF_POINTS)) && (!((DTYPE==1||CTYPE==1)&&OTYPE==0)))
      dout = din;
   else {
      if (L_START_LSB > 0) {  // LSB method
         if (L_METHOD == 1) { // rounding up
            dout = int96add(din,int96pow2(L_START_LSB-1,din.s));
            if (din.s == true) dout = int96add(dout,int96pow2(0,false));
         } else if (L_METHOD == 2) { // rounding away from zero
            dout = int96add(din,int96pow2(L_START_LSB-1,din.s));
         } else if (L_METHOD == 3) { // rounding forwards zero
            dout = int96add(din,int96pow2(L_START_LSB-1,din.s));
            if (din.s == true) dout = int96add(dout,int96pow2(0,false));
            else               dout = int96add(dout,int96pow2(0,true));
         } else if (L_METHOD == 4) { // convergent rounding
            if (L_START_LSB < 32) {
               tempp = 1<<L_START_LSB;
               if ((din.l & tempp) == 0) even_odd = true;
               else                      even_odd = false;
            } else if (L_START_LSB == 32) {
               if ((din.m & 0xfffffffe) == 0) even_odd = true;
               else                           even_odd = false;
            } else if (L_START_LSB < 64) {
               tempp = 1<<(L_START_LSB-32);
               if ((din.l & tempp) == 0) even_odd = true;
               else                      even_odd = false;
            } else if (L_START_LSB == 64) {
               if ((din.m & 0xfffffffe) == 0) even_odd = true;
               else                           even_odd = false;
            } else {
               tempp = 1<<(L_START_LSB-64);
               if ((din.l & tempp) == 0) even_odd = true;
               else                      even_odd = false;
            }
            if (din.s == false) {
               if (even_odd == true) {
                  dout = int96add(din,int96pow2(L_START_LSB-1,false));
                  dout = int96add(dout,int96pow2(0,true));
               } else {
                  dout = int96add(din,int96pow2(L_START_LSB-1,false));
               }
            } else {
               if (even_odd == true) {
                  dout = int96add(din,int96pow2(L_START_LSB-1,true));
                  dout = int96add(dout,int96pow2(0,false));
               } else {
                  dout = int96add(din,int96pow2(L_START_LSB-1,true));
               }
            }
         } else {
            if (din.s == true) {
               dout.h = 0xFFFFFFFF-din.h;
               dout.m = 0xFFFFFFFF-din.m;
               dout.l = 0xFFFFFFFF-din.l;
               dout.s = din.s;
               dout = int96add(dout,int96pow2(0,true));
            } else
               dout = din;
         }
         if (L_START_LSB < 32) {
            dout.l = dout.l >> L_START_LSB;
            a = dout.m << (32-L_START_LSB);
            dout.l = dout.l | a;
            dout.m = dout.m >> L_START_LSB;
            a = dout.h << (32-L_START_LSB);
            dout.m = dout.m | a;
            dout.h = dout.h >> L_START_LSB;
            if (L_METHOD == 0 && dout.s == true) {
               dout.h = 0xFFFFFFFF - (1<<(32-L_START_LSB)) + 1 + dout.h;
            }
         } else if (L_START_LSB == 32) {
            dout.l = dout.m;
            dout.m = dout.h;
            dout.h = 0;
            if (L_METHOD == 0 && dout.s == true) {
               dout.h = 0xFFFFFFFF;
            }
         } else if (L_START_LSB < 64) {
            dout.l = dout.m  >> (L_START_LSB - 32);
            a = dout.h << (64 - L_START_LSB);
            dout.l = dout.l | a;
            dout.m = dout.h >> (L_START_LSB - 32);
            dout.h = 0;
            if (L_METHOD == 0 && dout.s == true) {
               dout.m = 0xFFFFFFFF - (1<<(64-L_START_LSB)) + 1 + dout.m;
               dout.h = 0xFFFFFFFF;
            }
         } else {
            dout.l = dout.h >> (L_START_LSB - 64);
            dout.m = dout.h = 0;
            if (L_METHOD == 0 && dout.s == true) {
               dout.l = 0xFFFFFFFF - (1<<(96-L_START_LSB)) + 1 + dout.l;
               dout.m = 0xFFFFFFFF;
               dout.h = 0xFFFFFFFF;
            }
         }
         if (dout.s == true && L_METHOD == 0) {
            dout.h = 0xFFFFFFFF-dout.h;
            dout.m = 0xFFFFFFFF-dout.m;
            dout.l = 0xFFFFFFFF-dout.l;
            dout = int96add(dout,int96pow2(0,true));
         }

      } else {
         dout = din;
      }
      if (dout.s == true && dout.h == 0 && dout.m == 0 && dout.l == 0) {
         dout.s = false;
      }
      if (IDOUTWIDTH < L_FULL_WIDTH - L_START_LSB) { // MSB method
         temp = dout;
         LSFWIDTH = ((DTYPE==true || CTYPE==true) && OTYPE==true) ? IDOUTWIDTH-1:IDOUTWIDTH; // add dout_type
         if (LSFWIDTH >= 64) {
            temp.h = temp.h & (0xFFFFFFFF - ((1<<(LSFWIDTH-64))-1));
            temp.m = temp.l = 0;
         } else if (LSFWIDTH >= 32) {
            temp.m = temp.m & (0xFFFFFFFF - ((1<<(LSFWIDTH-32))-1));
            temp.l = 0;
         } else {
            temp.l = temp.l & (0xFFFFFFFF - ((1<<(LSFWIDTH))-1));
         }
         if (M_METHOD == 0) { // MSB method is saturation
            if (temp.h != 0 || temp.m != 0 || temp.l != 0) {
               if (dout.s == true)
                  dout.h = dout.m = dout.l = 0;
               else if (LSFWIDTH >= 64) {
                  dout.h = (1<<(LSFWIDTH-64))-1;
                  dout.m = dout.l = 0xFFFFFFFF;
               } else if (LSFWIDTH >= 32) {
                  dout.h = 0;
                  dout.m = (1<<(LSFWIDTH-32))-1;
                  dout.l = 0xFFFFFFFF;
               } else {
                  dout.h = dout.m = 0;
                  dout.l = (1<<LSFWIDTH)-1;
               }
            }
            if(OTYPE==false && dout.s==true) { // if negative value, add by mervin
               dout.h = 0;
               dout.m = 0;
               dout.l = 0;
               dout.s = false;
            }
         } else {
            if (LSFWIDTH >= 64) {
                dout.h = dout.h & ((1<<(LSFWIDTH-64))-1);
                dout.m = dout.m;
                dout.l = dout.l;
            } else if (LSFWIDTH >= 32) {
                dout.h = 0;
                dout.m = dout.m & ((1<<(LSFWIDTH-32))-1);
                dout.l = dout.l;
            } else {
                dout.h = dout.m = 0;
                dout.l = dout.l & ((1<<(LSFWIDTH))-1);
            }
         }
      } else { //IDOUTWIDTH == L_FULL_WIDTH - L_START_LSB
            if(OTYPE==false && dout.s==true) { // if negative value, add by mervin
               dout.h = 0;
               dout.m = 0;
               dout.l = 0;
               dout.s = false;
            }
      }
   }
   if(L_DOUT_POINTS>(L_DIN_POINTS+L_COEFF_POINTS)) {
      LSFW = (L_DOUT_POINTS-(L_DIN_POINTS+L_COEFF_POINTS));
      if(LSFW<32) {
         doutr.h = dout.h << LSFW;
         doutr.h = doutr.h + (dout.m>>(32-LSFW));
         doutr.m = dout.m << LSFW;
         doutr.m = doutr.m + (dout.l>>(32-LSFW));
         doutr.l = dout.l << LSFW;
      } else if(LSFW==32) {
         doutr.h = dout.m;
         doutr.m = dout.l;
         doutr.l = 0;
      } else {
         doutr.h = dout.m >> (LSFW-32);
         doutr.h = doutr.h + (dout.l >> (64-LSFW));
         doutr.m = (dout.l<<(32-LSFW));
         doutr.l = 0;
      }
      doutr.s = dout.s;//add by v2.1
      return doutr;
   } else {
      return dout;
   }
}

static sint64 int962sint64(int96 din) {
   sint64 dout;
   if(din.s==false) {
      dout = (sint64)din.m;
      dout = (dout<<32) + din.l;
   } else {
      dout = (sint64)din.m;
      dout = (dout<<32) + din.l;
      dout = ~dout;
      dout = dout + 1;
   }
   return dout;
}

scaler::scaler(int argc, char *argv[])
{
   NUM_FRAME      = 1;
   NUM_PLANE      = 1;
   PARALLEL       = 0;
   YCBCR422       = 0;
   YCBCR444       = 0;
   VINWIDTH       = 704;
   VINHEIGHT      = 480;
   VOUTWIDTH      = 704;
   VOUTHEIGHT     = 480;
   KERNEL         = 0;
   VFCWIDTH       = 32;
   VFCBPWIDTH     = 16;
   HFCWIDTH       = 32;
   HFCBPWIDTH     = 16;
   VFBVALUE       = (sint64)1<<VFCBPWIDTH;
   HFBVALUE       = (sint64)1<<HFCBPWIDTH;
   VDFACTOR       = ((VFBVALUE*VINHEIGHT)/VOUTHEIGHT);
   HDFACTOR       = ((HFBVALUE* VINWIDTH)/VOUTWIDTH);
   VFTAPS         = 4;
   HFTAPS         = 4;
   VFPHASES       = 256;
   HFPHASES       = 256;
   DYNAMIC        = 0;
   SEPPCLK        = 0;
   PAD_TYPE       = 1;// force to copy mode
   PAD_VALUE      = 0;
   RESAMPLE       = 2;
   MULTICYCLE     = 1;
   DIN_WIDTH      = 8;
   DIN_TYPE       = false;
   DIN_POINTS     = 0;
   COEFF_WIDTH    = 8;
   COEFF_TYPE     = true;
   COEFF_POINTS   = COEFF_WIDTH-1;
   MOUT_WIDTH     = 8;
   MOUT_POINTS    = 0;
   DOUT_WIDTH     = 8;
   DOUT_TYPE      = false;
   DOUT_POINTS    = 0;
   PBUSWIDTH      = 32;
   PADDRWIDTH     = 5;
   VCBUFFER       = 1;
   HCBUFFER       = 1;
   ADAPTIVE       = 0;
   CBANKS         = 1;
   CSETS_CS       = 0;
   EDGE_TH        = 8*(1<<(DIN_WIDTH-8));
   FSCALE         = 0.35;
   SHARE_CMEM     = 0;
   MULTTYPE       = 1;
   HIGHSPEED      = 0;
   LBUFFER        = 1;
   MSB_METHOD     = 0;
   LSB_METHOD     = 4;
   TAGS_WIDTH     = 0;
   FRMPORTS       = 0;
   RESYNC         = 0;
   CE             = 1;
   SR             = 1;
   MAX_CBANKS     = 4;
   TOTAL_INPUT    = 0;
   TOTAL_OUTPUT   = 0;
   PRE_EVEN_CS    = -1;
   SEED           = 0;
   hmem           = NULL;
   dmem           = NULL;
   gmem           = NULL;
   CUR_FRMWIDTH[0] = -1;
   CUR_FRMHEIGHT[0]= -1;
   CUR_OUTWIDTH[0] = -1;
   CUR_OUTHEIGHT[0]= -1;
   CUR_FRMWIDTH[1] = -1;
   CUR_FRMHEIGHT[1]= -1;
   CUR_OUTWIDTH[1] = -1;
   CUR_OUTHEIGHT[1]= -1;
   CUR_FRMWIDTH[2] = -1;
   CUR_FRMHEIGHT[2]= -1;
   CUR_OUTWIDTH[2] = -1;
   CUR_OUTHEIGHT[2]= -1;
}


int main(int argc, char *argv[])
{

   int STATUS = false;

   scaler *scaler_inst = NULL;
   scaler_inst = new scaler(argc, argv);
#if 0
   argc = 4;
   char* argv_1 = "Scaler_IP";
   //char* argv_2 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows/Debug/ImgScaler.lpc";
   char* argv_2 = "C:/Mausam/Projects/Lattice 2D Scaler IP/Rituj/windows_MJ_4/Debug/Scaler_1.ldc";
   //char* argv_3 = "-d"; //DEBUG //GENGOLDEN
   //char* argv_3 = "-g"; //GENGOLDEN
   //char* argv_3 = "-l"; //GENGOLDEN //LOADSTIMULUS 
   char* argv_3 = "-b"; //GENGOLDEN //LOADSTIMULUS //LOADBMPFILE
   //char* argv_3 = "-y";   //GENGOLDEN //LOADSTIMULUS //LOADYUVFILE

   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_MJ/Debug/peppers.bmp";
   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_MJ/Debug/Nearest_INPUT_YCBCR422.bmp";
   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_MJ/Debug/nearest.bmp";
   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_MJ/Debug/RGB_4k.bmp";
   char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/R
   ituj/windows_MJ_4/Debug/YCBCR422_4k.bmp";
   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_MJ/Debug/INPUT_YCBCR444.png";
   //char* argv_4 = "C:/Mausam/Projects/Lattice 2D Scaler IP/windows_LDC_l_mode/Debug/stimulusinputfile.dat";

   argv[0] = argv_1;
   argv[1] = argv_2;
   argv[2] = argv_3;
   argv[3] = argv_4;
#endif
   STATUS = scaler_inst->get_argu(argc, argv);
   if (STATUS == 1) {
     return 1;
   }

   //STATUS = scaler_inst->parselpc();
   STATUS = scaler_inst->parseldc();
   if (STATUS == 1) {
      printf("Failed to parse lpc file\n");
      return 1;
   }
   // allocate memory
   STATUS = scaler_inst->allocate_mem();
   if (STATUS == 1) {
      printf("Coefficient memory allocate error\n");
      return 1;
   } else if (STATUS == 2) {
      printf("Stimulus memory allocate error\n");
      return 2;
   } else if (STATUS == 3) {
      printf("Golden memory allocate error\n");
      return 3;
   }
   //STATUS = scaler_inst->rerand();

   // generate coefficient
   STATUS = scaler_inst->gen_coefficient();
   if (STATUS != 0) {
      printf("Failed to generate coefficient data\n");
      return 4;
   } 
   
   // generate rom/ram initial files for RTL coding
   STATUS = scaler_inst->regencoeff();
   
   // run SCALER;
   if (scaler_inst->GENGOLDEN) {
      STATUS = scaler_inst->run_scaling();
      if (STATUS != 0) {
         printf("Failed to generate stimulus data\n");
         return 5;
      }
   }
   //STATUS = scaler_inst->output_golden();
   //if (STATUS != 0) {
   //   printf("Failed to output golden data\n");
   //   return 7;
   //}

   /*if (scaler_inst->GENGOLDEN) {
      STATUS = scaler_inst->genparams();
      if (STATUS != 0) {
         printf("Failed to generate params file \n");
         return 8;
      }
      STATUS = scaler_inst->gen_orcapp();
      if (STATUS != 0) {
         printf("Failed to generate orcapp head file \n");
         return 9;
      }
   }*/

   if (scaler_inst != NULL) {
       delete scaler_inst;
       scaler_inst = NULL;
   }


   return STATUS;
}


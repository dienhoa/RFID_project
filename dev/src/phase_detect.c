  /**
 * Sample program using C convenience function to create an array of tag reads
 * and print the tags found.
 * @file readintoarray.c
 */

#include <tm_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

/* Enable this to use transportListener */
#ifndef USE_TRANSPORT_LISTENER
#define USE_TRANSPORT_LISTENER 0
#endif

#define usage() {errx(1, "Please provide reader URL, such as:\n"\
                         "tmr:///com4 or tmr:///com4 --ant 1,2\n"\
                         "tmr://my-reader.example.com or tmr://my-reader.example.com --ant 1,2\n");}

void errx(int exitval, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);

  exit(exitval);
}

void checkerr(TMR_Reader* rp, TMR_Status ret, int exitval, const char *msg)
{
  if (TMR_SUCCESS != ret)
  {
    errx(exitval, "Error %s: %s\n", msg, TMR_strerr(rp, ret));
  }
}

void serialPrinter(bool tx, uint32_t dataLen, const uint8_t data[],
                   uint32_t timeout, void *cookie)
{
  FILE *out = cookie;
  uint32_t i;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  for (i = 0; i < dataLen; i++)
  {
    if (i > 0 && (i & 15) == 0)
    {
      fprintf(out, "\n         ");
    }
    fprintf(out, " %02x", data[i]);
  }
  fprintf(out, "\n");
}

void stringPrinter(bool tx,uint32_t dataLen, const uint8_t data[],uint32_t timeout, void *cookie)
{
  FILE *out = cookie;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  fprintf(out, "%s\n", data);
}

void parseAntennaList(uint8_t *antenna, uint8_t *antennaCount, char *args)
{
  char *token = NULL;
  char *str = ",";
  uint8_t i = 0x00;
  int scans;

  /* get the first token */
  if (NULL == args)
  {
    fprintf(stdout, "Missing argument\n");
    usage();
  }

  token = strtok(args, str);
  if (NULL == token)
  {
    fprintf(stdout, "Missing argument after %s\n", args);
    usage();
  }

  while(NULL != token)
  {
    scans = sscanf(token, "%"SCNu8, &antenna[i]);
    if (1 != scans)
    {
      fprintf(stdout, "Can't parse '%s' as an 8-bit unsigned integer value\n", token);
      usage();
    }
    i++;
    token = strtok(NULL, str);
  }
  *antennaCount = i;
}

int ret_no_element(int32_t* arrayofphase)
{
  int i;
  int no_element = 0;
  for (i = 0;*(arrayofphase+i)!= '\0';i++){
    no_element++;
  }
  return no_element;
}

int main(int argc, char *argv[])
{
  TMR_Reader r, *rp;
  TMR_Status ret;
  TMR_Region region;
  TMR_ReadPlan plan;
  uint8_t *antennaList = NULL;
  uint8_t buffer[20];
  uint8_t i;
  uint8_t antennaCount = 0x0;
  TMR_String model;
  char str[64];
#if USE_TRANSPORT_LISTENER
  TMR_TransportListenerBlock tb;
#endif

  if (argc < 2)
  {
    usage();
  }

  for (i = 2; i < argc; i+=2)
  {
    if(0x00 == strcmp("--ant", argv[i]))
    {
      if (NULL != antennaList)
      {
        fprintf(stdout, "Duplicate argument: --ant specified more than once\n");
        usage();
      }
      parseAntennaList(buffer, &antennaCount, argv[i+1]);
      antennaList = buffer;
    }
    else
    {
      fprintf(stdout, "Argument %s is not recognized\n", argv[i]);
      usage();
    }
  }
  
  rp = &r;
  ret = TMR_create(rp, argv[1]);
  checkerr(rp, ret, 1, "creating reader");

#if USE_TRANSPORT_LISTENER

  if (TMR_READER_TYPE_SERIAL == rp->readerType)
  {
    tb.listener = serialPrinter;
  }
  else
  {
    tb.listener = stringPrinter;
  }
  tb.cookie = stdout;

  TMR_addTransportListener(rp, &tb);
#endif

  ret = TMR_connect(rp);
  checkerr(rp, ret, 1, "connecting reader");

 /* pcardaba */
  /* Begin ...
  region = TMR_REGION_NONE;
  ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
  checkerr(rp, ret, 1, "getting region");
  */
  printf("Setting region to EU3.\n");
  region = TMR_REGION_EU3;
  ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
  checkerr(rp, ret, 1, "setting region");

  {
    printf("Setting hoptable to 866300.\n");
    uint32_t hoptablelist[1] = {866300};//hoptable: 866300, 866900, 867500, 865700, 
    TMR_uint32List hoptable;
    hoptable.list = hoptablelist;
    hoptable.len = 1;
    hoptable.max = 1;
    ret = TMR_paramSet(rp, TMR_PARAM_REGION_HOPTABLE, &hoptable);
    checkerr(rp, ret, 1, "setting hoptable");
  }

  { // Setting Tari to 25uS
  TMR_GEN2_Tari tari = TMR_GEN2_TARI_25US; // should be 25 ????
  printf("Setting Tari to 25us\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TARI , &tari);      
  checkerr(rp, ret, 1, "setting tari");
  }

  { // Setting miller to 8
  TMR_GEN2_TagEncoding miller = TMR_GEN2_MILLER_M_8;
  printf("Setting miller to 8\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TAGENCODING , &miller);      
  checkerr(rp, ret, 1, "setting miller");
  }

  { // Setting Session to 1
  TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
  printf("Setting Session to 1\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_SESSION , &session);      
  checkerr(rp, ret, 1, "setting Session");
  }
  {
    TMR_GEN2_LinkFrequency blf = TMR_GEN2_LINKFREQUENCY_250KHZ;
    printf("Setting BLF 250KHz \n");
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_BLF , &blf); 
    checkerr(rp, ret, 1, "setting backscatter link frequency");
  }
  {
    TMR_GEN2_Target targetvalue = TMR_GEN2_TARGET_A;
    printf("Setting Target to A\n");
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TARGET , &targetvalue); 
    checkerr(rp, ret, 1, "setting Target");
  }
  { // Setting Q static 0
  TMR_SR_GEN2_QStatic Q;
  uint8_t initQ = 0;
  Q.initialQ = initQ;
  printf("Setting Q Static to 0\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_Q , &Q);      
  checkerr(rp, ret, 1, "setting Q Static");
  }

  model.value = str;
  model.max = 64;
  TMR_paramGet(rp, TMR_PARAM_VERSION_MODEL, &model);
  if (((0 == strcmp("M6e Micro", model.value)) ||(0 == strcmp("M6e Nano", model.value)))
    && (NULL == antennaList))
  {
    fprintf(stdout, "Module doesn't has antenna detection support please provide antenna list\n");
    usage();
  }
  /**
  * for antenna configuration we need two parameters
  * 1. antennaCount : specifies the no of antennas should
  *    be included in the read plan, out of the provided antenna list.
  * 2. antennaList  : specifies  a list of antennas for the read plan.
  **/ 

  // initialize the read plan 
  ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
  checkerr(rp, ret, 1, "initializing the  read plan");

  /* Commit read plan */
  ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
  checkerr(rp, ret, 1, "setting read plan");

  {
    /* false -- Each antenna gets a separate record
     * true -- All antennas share a single record */
    bool value = true;
    ret = TMR_paramSet(rp, TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA,&value);
    checkerr(rp, ret, 1, "setting uniqueByAntenna");
  }

  {
    int32_t tagCount;
    int32_t ant_1_count;
    int32_t ant_2_count;
    int32_t phase_ant_1_sum; 
    int32_t phase_ant_2_sum; 
    int32_t average_delta_phase;
    int32_t phase_1_pr;
    int32_t phase_2_pr;
    TMR_TagReadData* tagReads;
    int32_t* ant_1_phase = NULL;
    int32_t* ant_2_phase = NULL;
    int i;
    printf("Phase Difference for tag 300833B2DDD9014000000000\n");// E2006316963EDAB165385F6A
    while(1)
    {
      ant_1_count = 0;
      ant_2_count = 0;
      phase_ant_1_sum = 0;
      phase_ant_2_sum = 0;
      average_delta_phase = 0;
      ant_1_phase = malloc(sizeof(int32_t)*20);
      ant_2_phase = malloc(sizeof(int32_t)*20);
      ret = TMR_readIntoArray(rp, 800, &tagCount, &tagReads);
      checkerr(rp, ret, 1, "reading tags");

      printf("%d tags found.\n", tagCount);

      for (i=0; i<tagCount; i++)
      { 
        TMR_TagReadData* trd = &tagReads[i];
        char epcStr[128];
        TMR_bytesToHex(trd->tag.epc, trd->tag.epcByteCount, epcStr);
        if (0x00 == strcmp("300833B2DDD9014000000000",epcStr))
        {
          if (trd->antenna == 1)
          {
            *(ant_1_phase+ant_1_count) = trd->phase;
            printf(" phase ant 1:%d \n", *(ant_1_phase+ant_1_count));
            ant_1_count++;
          }

          if (trd->antenna == 2)
          {
            *(ant_2_phase+ant_2_count) = trd->phase;
            printf(" phase ant 2:%d \n", *(ant_2_phase+ant_2_count));
            ant_2_count++;
          }
        }
      }
      int32_t num_element_1;
      int32_t num_element_2;
      int32_t min_num_element;

      num_element_1 = ret_no_element(ant_1_phase);
      num_element_2 = ret_no_element(ant_2_phase);
      min_num_element = (num_element_1<num_element_2)?num_element_1:num_element_2;
      int32_t delta_phase[min_num_element];
      printf("no tag found ant 1: %d \n", num_element_1);
      printf("no tag found ant 2: %d \n", num_element_2);
      printf("no tag MIN: %d \n", min_num_element);
      for(int32_t i =0;i<min_num_element;i++)
      {
        delta_phase[i]=*(ant_1_phase+i)-*(ant_2_phase+i);
        if(delta_phase[i]<-90)
        {
          delta_phase[i] = delta_phase[i] + 180;
        }
        else if(delta_phase[i] > 90 )
        {
          delta_phase[i] =  -180 + delta_phase[i];
        }
        printf("delta_phase: %d \n", delta_phase[i]);
      }
          // if (trd->antenna == 1)
          // {
          //   if (i<=1)
          //   {
          //     phase_1_pr = trd->phase;
          //   }
            
          //   if ((trd->phase - phase_1_pr < 60)&&(trd->phase - phase_1_pr > -60))
          //   {
          //     phase_ant_1_sum += trd->phase;
          //     ant_1_count++;
          //     phase_1_pr = trd->phase;
          //   }
          // }
          // if (trd->antenna == 2)
          // {
          //   if (i<=1)
          //   {
          //     phase_2_pr = trd->phase;
          //   }
            
          //   if ((trd->phase - phase_2_pr < 60)&&(trd->phase - phase_2_pr > -60))
          //   {
          //     phase_ant_2_sum += trd->phase;
          //     ant_2_count++;
          //     phase_2_pr = trd->phase;
          //   }

          // }
          // printf("%s", epcStr);
          // printf(" ant:%d", trd->antenna);
          // printf(" phase:%d ", trd->phase);
          // printf(" RSSI:%d ", trd->rssi);
          // printf(" frequency:%d", trd->frequency);
          // printf("\n");   
      //   }
      // }
          
      // if((ant_1_count!=0)&&(ant_2_count!=0))
      // {
      //   average_delta_phase = (int)(phase_ant_1_sum/ant_1_count) - (int)(phase_ant_2_sum/ant_2_count);
      //   if(average_delta_phase<-90)
      //   {
      //     average_delta_phase = average_delta_phase + 180;
      //   }
      //   else if(average_delta_phase > 90 )
      //   {
      //     average_delta_phase =  -180 + average_delta_phase;
      //   }
      //   // printf("antenna 1 count:%d\n", ant_1_count);
      //   // printf("antenna 2 count:%d\n", ant_2_count);
      //   // printf("Sum of Phase ant 1:%d\n", phase_ant_1_sum);
      //   // printf("Sum of Phase ant 2:%d\n", phase_ant_2_sum);
      //   printf("Average of Difference Phase:%d\n", average_delta_phase);           
      // }
      // else
      // {
      //   printf("Not Enough Data to calculate !\n");
      // }
      //   free(tagReads);//free dynamics allocated memory 
      

    }

  TMR_destroy(rp);
  return 0;
  }
}

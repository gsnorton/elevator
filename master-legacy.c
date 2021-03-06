/*******************************************************************************************************
      Denise Cole  & Greg Norton  team
      Spring 2013 CS466 w/Ken Wade, inst
      Lab 7
      Task-master-to-test-with-10 cycles.c
      Started with code uCOSDemo2.c  Z-World, 2000

This program does the following:
A queue is defined to hold the alphabet, repeatedly. A technique is used such
that the queue is being written to and read from continuously by 2 Tasks.

Task 1: Drive motor.

Task 2: Unused


********************************************************************************************************/
#class auto          // Change default storage class for local variables: on the stack


/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-1999, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                                 V2.51
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               CONFIGURATION
*********************************************************************************************************
*/

// Redefine uC/OS-II configuration constants as necessary
#define OS_MAX_EVENTS          2       // Maximum number of events (semaphores, queues, mailboxes)
#define OS_MAX_TASKS           4       // Maximum number of tasks system can create (less stat and idle tasks)
#define OS_SEM_EN					 1			// Enable semaphore usage
#define OS_MBOX_EN             1       // Enable mailboxes
#define OS_MBOX_ACCEPT_EN      1
#define OS_MBOX_POST_EN        1
#define OS_TASK_CREATE_EN      0       // Disable normal task creation
#define OS_TASK_CREATE_EXT_EN  1       // Enable extended task creation
#define OS_TASK_STAT_EN        0       // Enable statistics task creation
#define OS_TIME_DLY_HMSM_EN    1       // Enable OSTimeDlyHMSM
#define STACK_CNT_512          4       // number of 512 byte stacks (application tasks + stat task + prog stack)
#define TRUE                   1
#define FALSE                  0
#define FORWARD                1
#define BACKWARD               0
#define FWD                 0x20
#define BKWD                0x30
#define HALT                0x00
#define MAX_CYCLE             10
#define MAX_LEVEL            248
#define MAX_SEQ                8
#define OS_TICKS_PER_SEC     682

#use "BL4S1xx.LIB"
#use "ucos2.lib"
#use "rtclock.lib"
#use "rand.lib"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of bytes)       */
#define          TASK_1_ID           1
#define          TASK_2_ID           2

#define          TASK_1_PRIO         5
#define          TASK_2_PRIO         6
/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

      		OS_EVENT    *My_Semaphore, *RandomSem;
            OS_EVENT    *T_Mbox;

 static int delayTics = 0;                    /* a small test to cause a variable time context switch */
        int channel;
        int channel_bank = 0x09;
        int m_seq[8] = {
                       		0x21,0x25,0x24,0x26,0x22,0x2A,0x28,0x29    //using lower order bits
                       };
        char taskpdata = 'R';
        int *f_ptr, *b_ptr, *s_ptr, *d_ptr;
 static struct tm  rtc;  // time structs

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void     Task1(void *data);
        void     Task2(void *pdata);

        void     ClearScreen();
        void     DispStr(int x, int y, char *s);
        char     Run_Motor_Seq(int pause_event, int curr_cycle, int pause_cycle, int pause_level);
        void     Run_Cont(void);
        void 	  calc_delay(struct tm rtc);
/*
*********************************************************************************************************
*                                                    FUNCTIONS
*********************************************************************************************************
*/
//---------------------------------Begin-Calc-the-Time-------------------------
void calc_delay(struct tm rtc)
{
    	struct tm  nrtc;  // time structs
    	int diff_hours, diff_mins, diff_secs, i, j;
    	char time_str[80];

      tm_rd(&nrtc);
 //		strftime( time_str, sizeof time_str, "E-Time = %I:%M:%S \n\n", &nrtc);
 //     DispStr(45,4,time_str);

      diff_hours = nrtc.tm_hour - rtc.tm_hour;   //assume same day
		diff_mins  = nrtc.tm_min  - rtc.tm_min;
   	diff_secs  = nrtc.tm_sec  - rtc.tm_sec;

   	if(diff_hours < 0)
   	{
   		printf("error- you are on a different day\n");
   	}
   	if((diff_mins<0)&&(diff_hours >=0))  //adjust hours
   	{
     	  	diff_hours -=1;
      	diff_mins = 60+diff_mins;
   	}
    	if((diff_secs<0)&&(diff_hours >=0)) //adjust mins
   	{
     		diff_mins -=1;
      	diff_secs = 60+diff_secs;
   	}
      if(diff_hours==0)
      {
      	if(diff_mins==0)
         {
         	printf("\n                                             Delay time is %i sec\n",diff_secs );
         }
         else //there are min to print
         	printf("\n                                             Delay time is  %i min %i sec\n",diff_mins, diff_secs );
}
      else //there are hours to print
        		printf("\n                                             Delay time is  %i hr %i min %i sec\n",diff_hours, diff_mins, diff_secs );
}//end calc_delay

//-----------------------------------End-Calc-the-Time-------------------------
char Run_Motor_Seq(int pause_event, int curr_cycle, int pause_cycle, int pause_level)
{
   int l,m,s, *s_ptr, *d_ptr;
   int l_counter;
   static char t2_msg='1';

   	s_ptr = &m_seq[0];
      m = 0;
      for(l=1;l<=MAX_LEVEL;l++)
      {
         channel_bank = *(s_ptr+(m++)%MAX_SEQ);
         digOutBank(0,channel_bank);
         OSTimeDly(1);
         if((l==pause_level)&&(curr_cycle==pause_cycle)&& pause_event)
         {
            digOutBank(0, (0x0F&channel_bank));
         	OSMboxPost(T_Mbox,(void*)&t2_msg);

         	OSTimeDly(682*5);//1 sec delay
         }//end pause event

         s_ptr = &m_seq[0];	//reset pointer to write the array again
      }//end levels forward
//.............................................Reverse........................
      s_ptr = &m_seq[MAX_SEQ-1];
      m = 0;
      for(l=MAX_LEVEL+1;l<=MAX_LEVEL*2;l++)
      {
         channel_bank = *(s_ptr+(m--)%MAX_SEQ);
         digOutBank(0,0x30 | channel_bank);  //mask slave command
	      if((l==pause_level)&&(curr_cycle==pause_cycle)&& pause_event)
	      {
	         digOutBank(0, (0x0F&channel_bank));
	         OSMboxPost(T_Mbox,(void*)&t2_msg);
	         OSTimeDly(682*5);//1 sec delay
	      }//end pause event
			else OSTimeDly(1);
      	s_ptr = &m_seq[MAX_SEQ-1];  //reset pointer to write the array again
      }//end levels backward
      digOutBank(0, (0x0F&channel_bank));
      if(pause_event)
      	return '1';
      else
      	return('2');
} //end run motors
//-----------------------------------Run_Cont---------------------------------
void Run_Cont(void)      //runs a complete cycle, then checks for a kbhit
{
   	int p,l, *r_ptr;

   		r_ptr = &m_seq[0];
      	p = 0;

       	for(l=0;l<MAX_LEVEL;l++)
         {
         		channel_bank = *(r_ptr+(p++)%MAX_SEQ);
            	digOutBank(0,channel_bank);
            	OSTimeDly(1);
         }//end for 'l' forward

         r_ptr = &m_seq[MAX_SEQ-1]; //reset pointer to write the array again

         p = 0;
         for(l=0;l<MAX_LEVEL;l++)
         {
         	channel_bank = *(r_ptr+(p--)%MAX_SEQ);
            digOutBank(0,0X30 | channel_bank);   //mask slave command
            OSTimeDly(1);
         }//end for 'l' backward
         r_ptr = &m_seq[MAX_SEQ-1];
} //end run cont/*
/********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
   ClearScreen();                                         /* Clear the screen                         */

   brdInit();
      // Configure all outputs to be general digital outputs that are low
   for(channel = 0; channel < BL_DIGITAL_OUT; ++channel)
   {
      setDigOut(channel, 0);                                // send all zeros to all the channels
   }
   	digOutBank(0, (0x0F&channel_bank));  //halt
   OSInit();                                              /* Initialize uC/OS-II                      */
   My_Semaphore = OSSemCreate(1);
   RandomSem 	 = OSSemCreate(1);
   T_Mbox       = OSMboxCreate((void *)0);

/*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/
    OSTaskCreateExt(Task1,
                   (void *)0,
                   TASK_1_PRIO,
                   TASK_1_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task2,
                  (void *)&taskpdata,
                   TASK_2_PRIO,
                   TASK_2_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
} //end main
/*
*********************************************************************************************************
*                                               TASK #1
*
* Description:
*********************************************************************************************************
*/

void  Task1 (void *pdata)
{
     	char time_str[80];
		char  key;
      int pause_cycle, pause_level;
      int pause_event;
    	int i, j;

   	tm_rd(&rtc);
      strftime( time_str, sizeof time_str, "Sys-Task Time = %I:%M:%S \n", &rtc); // add formatted hours/min/sec to the string
      DispStr(5,2,time_str);

     	printf("     Pause Event - 1\n");
   	printf("     2 min test  - 2\n");
   	printf("     Halt-Stop   - h\n\n");

      while(!kbhit()) {OSTimeDly(1);}

   	key = getchar();     // wait for a key

		while(1)
   	{
      	if(key == '1'|| key == '2')     //forward
         {
            if(key == '1')
            {
            	pause_event = 1;

    				printf("\n     Enter the Pause Cycle 1 - %i >> ", MAX_CYCLE);
            	while(!kbhit()) {OSTimeDly(1);}
      			scanf("%i",&pause_cycle);
					printf("%i\n",pause_cycle);
    				printf("\n     Enter the Pause Level Including\n");
               printf("\n     UP^vDOWN (1 - %i) >> ", MAX_LEVEL*2);
   				while(!kbhit()) {OSTimeDly(1);}
      			scanf("%i",&pause_level);
					printf("%i \n",pause_level);
            }
            else
            	pause_event = 0;


           	tm_rd(&rtc);  //gets pointer to the structure of time read
//            strftime( time_str, sizeof time_str, "S-Time = %I:%M:%S \n", &rtc); // add formatted hours/min/sec to the string
//            DispStr(45,2,time_str);
//-----------------------------------------------------Cycle-Loop--------------

            if(pause_event)
            {

            for(i=1;i<=MAX_CYCLE;i++)
            	{
//-----------------------------------------------------Level-Loops------------
						key = Run_Motor_Seq(pause_event, i, pause_cycle, pause_level);
//-----------------------------------------------------------------------------
             	}//end for - MAX_CYCLE loop
            digOutBank(0, (0x0F&channel_bank));      //send halt to slave
            }//end if pause_event
//-----------------------------------------------------------------------------
             else if(!pause_event)
             {
               	while(1)
                  {
                  	Run_Cont();
	                  if(kbhit())
	                  {
	                     key = getchar();
	                     if(key == 'H' || key == 'h')
	                     {
	                        digOutBank(0, (0x0F&channel_bank));  //halt
	                        break;
	                     }//end if k=h
                  	}//end if kbhit
                  }//end while
              }//end else if !pause_event
//--------------------------------------------------End Run Op 1&2-------------
              calc_delay(rtc);
              digOutBank(0, (0x0F&channel_bank));      //send halt to slave
			}//end if '1'||'2'
//-----------------------------------------------------------------------------
         else if(key == 'H' || key == 'h')      //Halt
         {
         	digOutBank(0, (0x0F&channel_bank));
          }
         else if (key == 'C' || key == 'c')
         {
            ClearScreen();
            DispStr(5,1,"Select 1, 2, C, ,H \n\n");
          }
         else
         {
            DispStr(5,1,"Wrong Selection - Select 1, 2, C, ,H \n\n");
            key = getchar();
          } //end else
//---------------------------Get-Next-KBD-CMD----------------------------------
          while(!kbhit())
          {
          	DispStr(5,1,"Select 1, 2, C, ,H \n\n");
            OSTimeDly(1);
          }
          if(kbhit())
          {
          	key = getchar();
          }
//---------------------------Loop-With-Next-KBD-CMD----------------------------
      }//end the main while forever run the motors

} //end Task1
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description:
*********************************************************************************************************
*/
//*********************************************************************************************************
void  Task2 (void *pdata)
{
    auto UBYTE x;
    auto UBYTE y;
    auto UBYTE err;
	 auto UBYTE num[2];
    OS_MBOX_DATA *t_msg;

    pdata = pdata;

    while(1)
    {
        OSSemPend(RandomSem, 0, &err);           // Acquire semaphore to perform random numbers
        x = rand16_range(1,5);                  // Find X position where number will appear
        y = rand16_range(1,5);                  // Find Y position where number will appear

//      sprintf(num, "%c", *((char *)pdata));
//		  DispStr(x + 59, y + 45, num);	          // Display the task number on the screen

        x = rand16_range(1,5);                  // Find X position where number will appear
        y = rand16_range(1,5);                  // Find Y position where number will appear

//		  DispStr(x + 59, y + 45, " ");
        OSSemPost(RandomSem);                    // Release semaphore

        t_msg = OSMboxAccept(T_Mbox);
        if(t_msg != (void*)1)
        {
       		digOut(7,1);        //send blink to an LED
//      putchar(0x07);
        }
        OSTimeDly(1);
    }//end while
}//end Task2/*

/*********************************************************************************************************
*                                      HELPER FUNCTIONS
*********************************************************************************************************
*/

nodebug void ClearScreen()
{
   printf ( "\x1Bt" );              // Space Opens Window
}

nodebug void DispStr (int x, int y, char *s)     //prints col and row x = col, y = row
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}



/*
 * PLCMITSU.c
 *
 *  Created on: Nov 2, 2018
 *      Author: Itachi
 */

#include "PLC_MITSU.h"

#define UART_TIMEOUT	10

extern UART_HandleTypeDef huart1;

/* --------------------------Read_D------------------------------
 * Read D memory of PLC FX series
 * Read from D address (ui16Start_Addr) up to D1023 (maximum)
 * Data frame structure collect from Autobase
 * Input:
 *  	   - ui16DataD_PLC: Pointer to array store data
 * 		   - ui16Start_Addr: Start address (Address want to read)
 *         - ui16Length: Size to read in Words
 * Output: True if finish the reading successful
---------------------------------------------------------------*/
bool Read_D(uint16_t *ui16DataD_PLC, //TODO
            uint16_t  ui16Start_Addr,
            uint16_t  ui16Length)
{
	/*-----Variables for PLC Read------- */
//    static uint16_t data_d_plc[50]={0}; //TODO - WHY STATIC
    uint16_t data_d_plc[50]={0};
    uint32_t csum=0;				//Check sum local variable
    uint8_t  csum_low,csum_high;
    uint8_t  type_low,type_high;
    uint8_t  size_low,size_high;	//Size of byte to read
    uint16_t start_addr_temp=0;
    uint8_t  start_addr_b0,start_addr_b1,start_addr_b2;
    bool retval=0;
    //Data processing
    //-------------Packet the Type (Read D Memory)----------------------//
    type_low    =   Convert_2Char(D_MEM_READ&0x0F);
    type_high   =   Convert_2Char((D_MEM_READ>>4)&0x000F);
    //-------------Packet the Start Address----------------------//
    start_addr_temp=     ui16Start_Addr*2;   //*2 because 2 byte (autobase data showing that)
    start_addr_b0=  Convert_2Char(start_addr_temp&0x0F);     start_addr_temp>>=4; //2 address per unit
    start_addr_b1=  Convert_2Char(start_addr_temp&0x0F);     start_addr_temp>>=4;
    start_addr_b2=  Convert_2Char(start_addr_temp&0x0F);     start_addr_temp>>=4;
    //-------------Packet number of word to READ----------------------//
    size_low    =   Convert_2Char( (ui16Length*2) &0x000F); //Read 1 word by default, (2 byte-autobase data showing that)
    size_high   =   Convert_2Char(( (ui16Length*2) >>4)&0x000F);
    //-------------Check sum----------------------//
    csum = type_high+type_low+start_addr_b2+start_addr_b1+start_addr_b0+size_high+size_low+ETX;
    csum = csum&0x00FF;                          // Collect 2 last nibbles for sum check
    csum_low = Convert_2Char(csum&0x000F);           // Last digit have  4 bits
    csum_high= Convert_2Char((csum>>4)&0x000F);
    //Package the data
    data_send_2plc[0]=STX;
    data_send_2plc[1]=type_high;
    data_send_2plc[2]=type_low;
    data_send_2plc[3]=start_addr_b2;
    data_send_2plc[4]=start_addr_b1;
    data_send_2plc[5]=start_addr_b0;
    data_send_2plc[6]=size_high;
    data_send_2plc[7]=size_low;
    data_send_2plc[8]=ETX;
    data_send_2plc[9]=csum_high;
    data_send_2plc[10]=csum_low;
    // Send data_send_2plc to UART transmitter and send to PLC
    HAL_UART_Transmit(&huart1, data_send_2plc, (sizeof(data_send_2plc)/ sizeof(uint8_t)), UART_TIMEOUT);
    if (ProcessData(data_d_plc, D_MEM_READ,ui16Length)==1) //If true value, then copy value to right place
    {
    	memcpy(ui16DataD_PLC,data_d_plc,(sizeof(data_send_2plc)/ sizeof(uint8_t)));
    	retval=1;
    }
    return retval;
}

///* ------------------Read_M(uint16_t ui16Start_Addr)----------------
// * Operating: Read M memory of PLC FX series
// * Read from address (ui16Start_Addr) (bit)
// * Data frame structure collect from Autobase
// * Input:  Start address (Address want to read)
// * Output: Data value at ui16Start_Addr (uint8_t)
//---------------------------------------------------------------*/
//uint8_t Read_M(uint16_t ui16Start_Addr)
//{
//	//Calculate variables (SEND)
//	uint8_t  Addr;
//	uint8_t  addr_low,addr_high;
//	unsigned long base_addr=0;
//	uint8_t  start_addr_b0,start_addr_b1,start_addr_b2;
//	uint8_t  amount,amount_low,amount_high;
//	unsigned long csum=0;
//	uint8_t  csum_low,csum_high;
//	uint8_t  send_count=0;
//	//Calculate variables (RECEIVE)
//	uint8_t  offset=0;
//	uint8_t  return_type=0;
//	uint8_t  Bit_shift=0;         //Bit shift  for Read_M_MEM
//	uint8_t     Bit_return=0;        //Bit return for Read_M_MEM
//	uint16_t ui16Data_Read_M=0;
//
//	//******************  SEND DATA TO PLC *********************//
//
//	Addr= M_MEM_READ;
//	addr_low    =   Convert_2Char(Addr&0x0F);
//	addr_high   =   Convert_2Char((Addr>>4)&0x000F);
//	//Convert Start_add to 3 bytes of character to send to PLC
//	//Offset definition
//	if          (( ui16Start_Addr/10)<4)                            offset=0;
//	else if     (((ui16Start_Addr/10)>=4)  && ((ui16Start_Addr/10)<8))  offset=1;
//	else if     (((ui16Start_Addr/10)>=8)  && ((ui16Start_Addr/10)<12)) offset=2;
//	else if     (((ui16Start_Addr/10)>=12) && ((ui16Start_Addr/10)<16)) offset=3;
//	else if     (((ui16Start_Addr/10)>=16) && ((ui16Start_Addr/10)<20)) offset=4;
//	else if     (((ui16Start_Addr/10)>=20) && ((ui16Start_Addr/10)<22)) offset=5;
//	base_addr=  (( ui16Start_Addr/10)|0x100)+offset;
//	start_addr_b0=  Convert_2Char(base_addr&0x0F);     base_addr>>=4;
//	start_addr_b1=  Convert_2Char(base_addr&0x0F);     base_addr>>=4;
//	start_addr_b2=  Convert_2Char(base_addr&0x0F);     base_addr>>=4;
//	//Size of data
//	if ( ( (ui16Start_Addr/10)==12 ) || ((ui16Start_Addr/10)==19)) amount=0x03;
//	else              amount=0x02;
//	amount_low    =   Convert_2Char(amount&0x000F);
//	amount_high   =   Convert_2Char((amount>>4)&0x000F);
//	//Check sum
//	csum = addr_high+addr_low+start_addr_b2+start_addr_b1+start_addr_b0+amount_high+amount_low+ETX;
//	csum =      csum&0x00FF;                          // Collect 2 last nibbles for sum check
//	csum_low=   Convert_2Char(csum&0x000F);           // Last digit have  4 bits
//	csum_high=  Convert_2Char((csum>>4)&0x000F);
//	//Package the data
//	data_send_2plc[0]=STX;
//	data_send_2plc[1]=addr_high;
//	data_send_2plc[2]=addr_low;
//	data_send_2plc[3]=start_addr_b2;
//	data_send_2plc[4]=start_addr_b1;
//	data_send_2plc[5]=start_addr_b0;
//	data_send_2plc[6]=amount_high;
//	data_send_2plc[7]=amount_low;
//	data_send_2plc[8]=ETX;
//	data_send_2plc[9]=csum_high;
//	data_send_2plc[10]=csum_low;
//	// Send data_send_2plc to UART transmitter and send to PLC
//	for(send_count=0;send_count<11;send_count++)
//	{
//		UARTCharPut(UART1_BASE, data_send_2plc[send_count]);
////		delay_us(10);
//	}
////	delay_us(10000);
//	//****************** RECEIVE DATA FROM PLC *********************//
//	// When finish collect data from Process there will be a
//	// right shift to collect corresponding bit
//	return_type= (ui16Start_Addr/10)%4; //There are 4 type of return
//	Bit_shift=    ui16Start_Addr%10;         //Bit shift will be 0-9
////        ui16Data_Read_M=ProcessData(M_MEM_READ);
////        type0 - real data in bit 0-9 of 16bits
//	if (return_type==0)
//	{
//		ui16Data_Read_M=ui16Data_Read_M&0x3FF;
//	}
//	else if (return_type==1)
//	{
//		ui16Data_Read_M=(ui16Data_Read_M>>2)&0x3FF;
//	}
//	else if (return_type==2)
//	{
//		ui16Data_Read_M=(ui16Data_Read_M>>4)&0x3FF;
//	}
//	else if (return_type==3)
//	{
//		ui16Data_Read_M=(ui16Data_Read_M>>6)&0x3FF;
//	}
//	else;
//	Bit_return=(ui16Data_Read_M>>Bit_shift)&0x01;
//	return Bit_return;
//}

///* ---------Write_D(uint16_t ui16Start_Addr, uint16_t Value)---------
// * Operating: Write D memory of PLC FX series
// * write to D address  (word-2 bytes)
// * Data frame structure collect from Autobase
// * Input:  uint16_t Start_add(Address want to write), uint16_t Value(value to write)
// * Output: None
// * Write D address is 0x11
//----------------------------------------------------------------*/
//void Write_D(uint16_t ui16Start_Addr, uint16_t Value){
//    /*-----Local Variables ------ */
//        unsigned long csum=0;
//        uint8_t  csum_low,csum_high;
//        uint8_t  Addr,Amount;  //Change if want to configure different protocol
//        uint8_t  addr_low,addr_high;
//        uint8_t  amount_low,amount_high;
//        uint8_t  start_addr_b0,start_addr_b1,start_addr_b2;
//        uint8_t  send_count=0;
//        uint8_t  value_b0,value_b1,value_b2,value_b3;
//
//
//        //    Data processing
//        Addr        =   D_MEM_WRITE; //Change if want to connect to different memory address
//        addr_low    =   Convert_2Char(Addr&0x0F);
//        addr_high   =   Convert_2Char((Addr>>4)&0x000F);
//
//        ui16Start_Addr=     ui16Start_Addr*2;    //*2 because 2 byte (autobase data showing that)
//        start_addr_b0=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4; //2 address per unit
//        start_addr_b1=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4;
//        start_addr_b2=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4;
//
//        Amount=0x01*2;   //Read 1 word by default, (2 byte-autobase data showing that)
//        amount_low    =   Convert_2Char(Amount&0x000F);
//        amount_high   =   Convert_2Char((Amount>>4)&0x000F);
//
//        value_b0= Convert_2Char(Value&0x0F);     Value>>=4;
//        value_b1= Convert_2Char(Value&0x0F);     Value>>=4;
//        value_b2= Convert_2Char(Value&0x0F);     Value>>=4;
//        value_b3= Convert_2Char(Value&0x0F);     Value>>=4;
//
//        csum = addr_high+addr_low+start_addr_b2+start_addr_b1+start_addr_b0+amount_high+amount_low+value_b0+value_b1+value_b2+value_b3+ETX;
//        csum =      csum&0x00FF;                          // Collect 2 last nibbles for sum check
//        csum_low=   Convert_2Char(csum&0x000F);           // Last digit have  4 bits
//        csum_high=  Convert_2Char((csum>>4)&0x000F);
//        //Pack data and send
//        data_send_2plc[0]=STX;
//        data_send_2plc[1]=addr_high;
//        data_send_2plc[2]=addr_low;
//        data_send_2plc[3]=start_addr_b2;
//        data_send_2plc[4]=start_addr_b1;
//        data_send_2plc[5]=start_addr_b0;
//        data_send_2plc[6]=amount_high;
//        data_send_2plc[7]=amount_low;
//        data_send_2plc[8]=value_b1;
//        data_send_2plc[9]=value_b0;
//        data_send_2plc[10]=value_b3;
//        data_send_2plc[11]=value_b2;
//        data_send_2plc[12]=ETX;
//        data_send_2plc[13]=csum_high;
//        data_send_2plc[14]=csum_low;
//        for(send_count=0;send_count<15;send_count++)
//        {
//            UARTCharPut(UART1_BASE, data_send_2plc[send_count]);
//            delay_us(10);
//        }
//        delay_us(10000);
//    }

/* ------------Write_M(uint16_t ui16Start_Addr, uint8_t Type)------
 * Operating: Write M memory of PLC FX series
 * write to M address (ui16Start_Addr) (bit)
 * Data frame structure collect from Autobase
 * Input:  uint16_t Start_add(Address want to write)
		   uint8_t Type can be either M_SET or M_RESET
 * Output: None
 * Write M address is 0x0800
//---------------------------------------------------------------*/
void Write_M(uint16_t ui16Start_Addr, uint8_t Type)
{
	unsigned long csum=0;
	uint8_t  csum_low,csum_high;
	uint8_t  start_addr_b0,start_addr_b1,start_addr_b2,ui16Start_Addr_b3;
	// Data processing
	// Type can be SET or RESET
	Type=Convert_2Char(Type);
	ui16Start_Addr   =  ui16Start_Addr+0x0800;      // Write M address is 0x0800
	start_addr_b0=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4; //2 address per unit
	start_addr_b1=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4;
	start_addr_b2=  Convert_2Char(ui16Start_Addr&0x0F);     ui16Start_Addr>>=4;
	ui16Start_Addr_b3=  Convert_2Char(ui16Start_Addr&0x0F);
	csum = Type+ui16Start_Addr_b3+start_addr_b2+start_addr_b1+start_addr_b0+ETX;
	csum =      csum&0x00FF;                          // Collect 2 last nibbles for sum check
	csum_low=   Convert_2Char(csum&0x000F);           // Last digit have  4 bits
	csum_high=  Convert_2Char((csum>>4)&0x000F);
	//Package the data
	data_send_2plc[0]=STX;
	data_send_2plc[1]=Type;
	data_send_2plc[2]=start_addr_b1;
	data_send_2plc[3]=start_addr_b0;
	data_send_2plc[4]=ui16Start_Addr_b3;
	data_send_2plc[5]=start_addr_b2;
	data_send_2plc[6]=ETX;
	data_send_2plc[7]=csum_high;
	data_send_2plc[8]=csum_low;
	// Send data_send_2plc to UART transmitter and send to PLC
	HAL_UART_Transmit(&huart1, data_send_2plc, 9, 10);
	HAL_Delay(1000);
}


// Consider to use timer to start timeout, stay in this loop is very dangerous
uint8_t	ProcessData(uint16_t  *Data_PLC,
                    uint8_t    Mem_Type,
                    uint16_t   Size )
{
//    unsigned long time_out_max=0;
    uint16_t data_length;
    uint8_t recv_temp=0;         //variable to collect data from UART
    uint8_t start_process=0;     //function running flag
    uint8_t byte_count=0 ;       //increase after collect 1 byte data
    uint8_t byte_end=0;          //assign the last value of data frame "ETX"
    uint8_t csum_count_recv=0;   //increase after collect 1 byte checksum data
    uint8_t csum_count_cal=0;    //use to calculate check sum
    unsigned long sum_cal=0;
    uint8_t sum_low,sum_high;
    uint8_t data_correct=0;      //data receive flag, detect if data is right or wrong
    //Stay here as long as there are data in receive FIFO and within timeout
//    time_out_max=Size*7;
//    if (Size==50) time_out_max=250; //800us for each word
//    if (Size==20) time_out_max=20;
//    if (Size==10) time_out_max=10;
//    while((!data_correct) && (Timeout(time_out_max, 2)==0))
{
//    while(UARTCharsAvail(UART1_BASE))
    {
//        recv_temp=UARTCharGet(UART1_BASE);
 //-------------------Detect START signal-------------------------------//
        if(recv_temp==STX)
        {
            byte_count=0;                           //Reset counter
            start_process=1;                        //Start the processing
            data_recv[byte_count]=recv_temp;        //Store the STX signal (1st element)
            byte_count++;                           //Next byte
//            delay_us(2000);
        }

//--------If it is not the start or stop bit but start bit already appear, store data value-----------------//
        else if( (start_process==1) && (recv_temp!=ETX) )
        {
            data_recv[byte_count]=recv_temp;
            byte_count++;
//            delay_us(500);
        }
//-----------Detect STOP signal, start ending process, check sum and store value---------------------------//
        else if( (start_process==1) && (recv_temp==ETX))
        {
            data_recv[byte_count]=recv_temp;  //Store ETX signal (0x03) in DATA[byte_count]
            byte_end=byte_count;              //Detect how many bytes received
            data_length=(byte_end-1)/4;
            byte_count++;
//            delay_us(1000);
            // Collect check sum data from PLC
            for(csum_count_recv=0;csum_count_recv<2;csum_count_recv++)
            {
                //Careful with infinite loop below
//                while(UARTCharsAvail(UART1_BASE)==0); //If no data stay in loop, do next instruction when have data
//                data_recv[byte_count]=UARTCharGet(UART1_BASE);
                byte_count++;
            }
            // Calculate sum by receiving data
            for(csum_count_cal=1;csum_count_cal<=byte_end;csum_count_cal++)  sum_cal+=data_recv[csum_count_cal];   //Sum=data_recv[1->byte_end]
            sum_low =Convert_2Char(sum_cal&0x0F);             //sum_low (char)
            sum_high=Convert_2Char((sum_cal>>4)&0x0F);        //sum_high (char)
            // Compare data from PLC VS receiving data
//            if( (data_recv[byte_end+1]==sum_high) && (data_recv[byte_end+2]==sum_low))    { data_correct=1; break;}
//            else                                                                            data_correct=0;
        }
    }
}
//    UARTFIFODisable(UART1_BASE);
//    UARTFIFOEnable(UART1_BASE);
    if(data_correct==1)
    {
        uint8_t temp_data_count=0;
        uint8_t temp_data_count1=0;
        for(temp_data_count=0;temp_data_count<data_length;temp_data_count++)
        {
           Data_PLC[temp_data_count]=Calculate_Data(data_recv[(temp_data_count1+2)], data_recv[(temp_data_count1+1)], data_recv[(temp_data_count1+4)], data_recv[(temp_data_count1+3)]);
           temp_data_count1+=4;
//           Error_PLC=0;
        }
        return 1;
    }
//    Error_PLC++;
    return 0;
}
/* ---------uint16_t Correct_Process(uint16_t Mem_type)-------------
 * Operating: Find the true value of Process function
 * Because Process usually return 0, try (n=200) time if none zero return
 * After 200 time but there no data except 0-> real data =0 return 0
 * Input:  uint8_t Data memory type
 * Output: True value
 * Change: Change the time max_try to test, n bigger test more time
 *         -> Increase accuracy but take more time to execute
-------------------------------------------------------*/
uint16_t Correct_Process(uint8_t ui8Mem_type)
{
    unsigned long max_try=3;
    unsigned long temp=0;
    unsigned long temp1=0;
    uint16_t  ui16Data=0;
    //Try 250 time to see if the data != 0, if not -> true data =0, if there is data =0
    //escape the loop and return true value
    for(temp=0;temp<=max_try;temp++)
        {
//           temp1=ProcessData(ui8Mem_type);
           if (temp1!=0)
               {
                   ui16Data=temp1;  //Backup data if true and exit
                   break;
               }
        }
    return ui16Data;                //return uint16_t data
}

/* ----Calculate_Data(char byte0,char byte1,char byte2,char byte3)---
 * Operating: Convert data receive from PLC to real value
 * Input:  Byte 0,1,2,3 of real data
 * Output: 16-bit Real data
--------------------------------------------------------------------*/
uint16_t Calculate_Data(char byte0,char byte1,char byte2,char byte3){
    uint16_t cal_data=0;
    byte0=Convert_2Numb(byte0);
    byte1=Convert_2Numb(byte1);
    byte2=Convert_2Numb(byte2);
    byte3=Convert_2Numb(byte3);
    cal_data=(byte3<<12)+(byte2<<8)+(byte1<<4)+byte0;
    return cal_data;
}

/* ----------Convert_2Numb(char char_in)-----------------
 * Operating: Convert character to number
 * Input:  character want to convert to number
 * Output: corresponding number
 * '0' - '9' -> 0 - 9
 * 'A' - 'F' -> 0x0A - 0x0F
-------------------------------------------------------*/
uint8_t  Convert_2Numb(char char_in){
    uint8_t numb_out;
    if(char_in<'A')  numb_out=char_in-0x30; //'0-9' return 0-9
    else             numb_out=char_in-55;   //'A'-'F' return 0x0A-0x0F
    return numb_out;
}

/* ---------Convert_2Char(uint8_t numb_in)--------------
 * Operating: Convert number to character
 * Input:  Number want to convert to char
 * Output: corresponding char character
 * 0 - 9 -> '0' -> '9'
 * 0x0A  ->0x0F return 'A'->'F'
-------------------------------------------------------*/
char Convert_2Char(uint8_t numb_in){
    char char_out;
    if(numb_in<0x0A)  char_out=numb_in+0x30; //0-9 return '0'-'9'
    else              char_out=numb_in+55;   //0x0A-0x0F return 'A'-'F'
    return char_out;
}



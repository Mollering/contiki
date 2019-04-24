//This is the library called for the msg struct
struct msg_to_send{

   uint16_t bat;
   int temp;
   rtimer_clock_t timestamp; // To save the light value
};

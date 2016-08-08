#define NEED_CONSTANTS

#include "usb.h"
#include <string.h>
#include <mystuff.h>

#ifdef USB_LOW_SPEED
#include "usb_table_1bit.h"
#else
#error NOT IMPLEMENTED
#include "usb_table_3bit.h"
#endif

struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));

//TODO: register this in http://pid.codes/howto/ or somewhere.

#define ENDPOINT0_SIZE 8

//Taken from http://www.usbmadesimple.co.uk/ums_ms_desc_dev.htm
static const uint8_t device_descriptor[] = {
	18, //Length
	1,  //Type (Device)
	0x10, 0x01, //Spec
	0x0, //Device Class
	0x0, //Device Subclass
	0x0, //Device Protocol  (000 = use config descriptor)
	0x08, //Max packet size for EP0 (This has to be 8 because of the USB Low-Speed Standard)
	0xcd, 0xab, //ID Vendor
	0x66, 0x82, //ID Product
	0x01, 0x00, //ID Rev
	1, //Manufacturer string
	2, //Product string
	0, //Serial string
	1, //Max number of configurations
};

static const uint8_t config_descriptor[] = {  //Mostly stolen from a USB mouse I found.
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	0x3b, 0x00,			// wTotalLength  	

	//34, 0x00, //for just the one descriptor
	
	0x01,					// bNumInterfaces (Normally 1)
	0x01,					// bConfigurationValue
	0x00,					// iConfiguration
	0x80,					// bmAttributes (was 0xa0)
	0x64,					// bMaxPower (200mA)


	//Mouse
	9,					// bLength
	4,					// bDescriptorType
	0,			// bInterfaceNumber (unused, would normally be used for HID)
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass
	0x02,					// bInterfaceProtocol (Mouse)
	0,					// iInterface

	9,					// bLength
	0x21,					// bDescriptorType (HID)
	0x10,0x01,		//bcd 1.1
	0x00, //country code
	0x01, //Num descriptors
	0x22, //DescriptorType[0] (HID)
	0x48, 0x00, //Descriptor length XXX This looks wrong!!!

	7, //endpoint descriptor (For endpoint 1)
	0x05, //Endpoint Descriptor (Must be 5)
	0x81, //Endpoint Address
	0x03, //Attributes
	0x04,	0x00, //Size
	0x10, //Interval (Was 0x0a)


	//Keyboard  (It is unusual that this would be here)
	9,					// bLength
	4,					// bDescriptorType
	1,			// bInterfaceNumber  = 1 instead of 0 -- well make it second.
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass
	0x01,					// bInterfaceProtocol (??)
	0,					// iInterface

	9,					// bLength
	0x21,					// bDescriptorType (HID)
	0x10,0x01,		//bcd 1.1
	0x00, //country code
	0x01, //Num descriptors
	0x22, //DescriptorType[0] (HID)
	63, 0x00, //Descriptor length XXX This looks wrong!!!

	7, //endpoint descriptor (For endpoint 1)
	0x05, //Endpoint Descriptor (Must be 5)
	0x83, //Endpoint Address
	0x03, //Attributes
	0x08,	0x00, //Size (8 bytes)
	0x10, //Interval (Was 0x0a)
};




static const uint8_t mouse_hid_desc[] = {  //Mostly stolen from a Microsoft USB mouse I found.
	0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03,
	0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x75, 0x05, 0x95, 0x01, 0x81, 0x01,
	0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x03,
	0x81, 0x06, 0xc0, 0x05, 0xff, 0x09, 0x02, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x01, 0xb1,
	0x22, 0x75, 0x07, 0x95, 0x01, 0xb1, 0x01, 0xc0, 
};

//From http://codeandlife.com/2012/06/18/usb-hid-keyboard-with-v-usb/
static const uint8_t keyboard_hid_desc[63] = {   /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};


#define STR_MANUFACTURER L"CNLohr"
#define STR_PRODUCT      L"ESPUSB"
#define STR_SERIAL       L"000"

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
};
const static struct usb_string_descriptor_struct string0 = {
	4,
	3,
	{0x0409}
};
const static struct usb_string_descriptor_struct string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
const static struct usb_string_descriptor_struct string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
const static struct usb_string_descriptor_struct string3 = {
	sizeof(STR_SERIAL),
	3,
	STR_SERIAL
};


// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
const static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
	{0x2200, 0x0000, mouse_hid_desc, sizeof(mouse_hid_desc)},
	{0x2200, 0x0001, keyboard_hid_desc, sizeof(keyboard_hid_desc)},
//	{0x2100, KEYBOARD_INTERFACE, config1_descriptor+KEYBOARD_HID_DESC_OFFSET, 9},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},	
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};
#define DESCRIPTOR_LIST_ENTRIES ((sizeof(descriptor_list))/(sizeof(struct descriptor_list_struct)) )



void handle_setup( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	ist->usb_buffer_status = 2;
	ist->setup_request = 1;

	//XXX TODO: Check device_address against address in this token.

	//If we get an "setup" token, we have to strike any accept buffers.
	ist->usb_bufferaccept = 0;

	__asm__ __volatile__( "movi a0, usb_reinstate" ); //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void handle_sof( uint32_t this_token, struct usb_internal_state_struct * ist )
{
}

void PerpetuatePacket( struct usb_internal_state_struct * ist )
{
	if( ist->usb_bufferret )
	{
		int tosend = ist->usb_bufferret_len;
		if( tosend > 8 ) tosend = 8;

		uint8_t sendnow[12];
		sendnow[0] = 0x80;
		if( ist->usb_buffer_status & 0x02 )
		{
			sendnow[1] = 0b01001011; //DATA1
		}
		else
		{
			sendnow[1] = 0b11000011; //DATA0
		}

		if( ist->usb_bufferret == (uint8_t*)1 )  //Tricky: Empty packet.
		{
			usb_send_data( sendnow, 2, 3 );  //Force a CRC
			ist->usb_bufferret = 0;
			ist->usb_bufferret_len = 0;
		}
		else
		{
			if( tosend )
				ets_memcpy( sendnow+2, ist->usb_bufferret, tosend );
			usb_send_data( sendnow, tosend+2, 0 );
			ist->last_sent_qty = tosend;
		}
	}
}

void handle_in( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;

	//If we get an "in" token, we have to strike any accept buffers.
	ist->usb_bufferaccept = 0;

	if( endp == 0x1 && addr == ist->my_address )
	{
		if( ist->sendep1 )
		{
			ist->usb_bufferret = ist->ep1data;
			ist->usb_bufferret_len = sizeof( ist->ep1data );
			ist->sendep1 = 0;
		}
		else
		{
			uint8_t sendword[2] = { 0x80, 0x5a };  //Empty data.
			usb_send_data( sendword, 2, 2 );
			return;
		}
	}

	if( endp == 0x3 && addr == ist->my_address )
	{
		if( ist->sendep2 )
		{
			ist->ep2data[0] = 0;
			ist->ep2data[1] = 0;
			ist->ep2data[2] = 8;
			ist->ep2data[3] = 0;
			ist->usb_bufferret = ist->ep2data;
			ist->usb_bufferret_len = sizeof( ist->ep2data );
			ist->sendep2 = 0;
		}
		else
		{
			uint8_t sendword[2] = { 0x80, 0x5a };  //Empty data.
			usb_send_data( sendword, 2, 2 );
			return;
		}
	}


	//TODO: Need to handle saying "nah, we're good!" if we're good.
	//00688a69
	PerpetuatePacket( ist );
}

void handle_out( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	//I ... uuhh... I don't think we need to do anything here, unless we're accepting interrupt_in data.

	__asm__ __volatile__( "movi a0, usb_reinstate" );  //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void handle_data( uint32_t this_token, struct usb_internal_state_struct * ist, uint32_t which_data )
{
	//Received data from host.
	//First, check to make sure this is the right data.
//	if( ist->packet_size < 4 ) return;

	//XXX TODO: Check to see if it is of nonzero length?
	if( (ist->usb_buffer_status&1) != which_data )
	{
		goto just_ack;
	}

	ist->usb_buffer_status ^= 1; //Expect opposite DATA command next.

	if( ist->usb_bufferaccept )  //For accepting data from endpoints and control out messages.
	{
		int acc = ist->packet_size-3;  //packet_size includes CRC and PID
		ist->debug = ist->packet_size;

		if( acc > ist->accept_length )
		{
			acc = ist->accept_length;
		}

		ets_memcpy( ist->usb_bufferaccept, ist->usb_buffer+1, acc );  //First byte of USB buffer is token.
		ist->accept_length -= acc;
		ist->usb_bufferaccept += acc;

		if( ist->accept_length == 0 )
		{
			ist->user_control_length_acc = ist->usb_bufferaccept - ist->user_control;
			ist->usb_bufferaccept = 0;
		}
	}

	if( ist->setup_request )
	{
		ist->setup_request = 0;
		struct usb_urb * s = (struct usb_urb *)ist->usb_buffer;
		//Flip endian on wValue and wLength.  Consider wIndex.
		//s->wValue = ((s->wValue)>>8) | ((s->wValue&0xff)<<8);
		//s->wLength = ((s->wLength)>>8) | ((s->wLength&0xff)<<8);

		//Send just a data packet.
		ist->usb_bufferret = (uint8_t*)1;
		ist->usb_bufferret_len = 0;

		if( s->bmRequestType & 0x80 )
		{
			if( s->bRequest == 0x06 ) //Get Request
			{
				int i;
				const struct descriptor_list_struct * dl;
				for( i = 0; i < DESCRIPTOR_LIST_ENTRIES; i++ )
				{
					dl = &descriptor_list[i];
					if( dl->wIndex == s->wIndex && dl->wValue == s->wValue )
						break;
				}

				if( i == DESCRIPTOR_LIST_ENTRIES )
				{
					//??? Somehow fail?  Is 'nak' the right thing? I don't know what to do here.
					goto just_ack;
				}


				ist->usb_bufferret = dl->addr;
				ist->usb_bufferret_len = dl->length;
				if( s->wLength < ist->usb_bufferret_len ) ist->usb_bufferret_len = s->wLength;
			}
			if( s->bRequest == 0x00 )
			{
				ist->usb_bufferret = (uint8_t*)&ist->status;
				ist->usb_bufferret_len = 3;
			}

			/////////////////////////////EXAMPLE CUSTOM CONTROL MESSAGE/////////////////////////////
			if(s->bRequest == 0xa7 ) //US TO HOST "in"
			{
				if( ist->user_control_length_ret )
				{					
					ist->usb_bufferret = ist->user_control;
					ist->usb_bufferret_len = ist->user_control_length_ret;
					if( s->wLength < ist->usb_bufferret_len ) ist->usb_bufferret_len = s->wLength;
					ist->user_control_length_ret = 0;
				}
			}
		}
		else if( s->bmRequestType == 0x00 )
		{
			if( s->bRequest == 0x05 ) //Set address.
			{
				ist->my_address = s->wValue;
			}
			if( s->bRequest == 0x09 ) //Set configuration.
			{
				//s->wValue; has the index.  We don't really care about this.
			}

			/////////////////////////////EXAMPLE CUSTOM CONTROL MESSAGE/////////////////////////////
			if( s->bRequest == 0xa6 && ist->user_control_length_acc == 0 ) //HOST TO US "out"
			{
				ist->usb_bufferaccept = ist->user_control;

				ist->accept_length = sizeof( ist->user_control );
				if( s->wLength < ist->accept_length )
					ist->accept_length = s->wLength;
			}
		}
	}


just_ack:
	{
		//Got the right data.  Acknowledge.
		uint8_t sendword[2] = { 0x80, 0xD2 };
		usb_send_data( sendword, 2, 2 );
	}
}

void handle_ack( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	ist->usb_buffer_status ^= 2;  //Invert next "DATAx" to host.

	if( ist->usb_bufferret > (uint8_t*)1 )
	{
		ist->usb_bufferret_len -= ist->last_sent_qty;
		if( ist->usb_bufferret_len == 0 )
			ist->usb_bufferret = 0;
		else
			ist->usb_bufferret += ist->last_sent_qty;
	}

	ist->last_sent_qty = 0;

//	PerpetuatePacket( ist );
}



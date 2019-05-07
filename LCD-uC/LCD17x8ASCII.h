/*
 * LCD17x8ASCII.h
 *
 * Created: 29.11.2012 12:24:39
 *  Author: operator
 */ 


#ifndef LCD17X8ASCII_H_
#define LCD17X8ASCII_H_

//_____ I N C L U D E S ________________________________________________________

#include <avr/pgmspace.h>

//_____ D E F I N I T I O N S __________________________________________________

//_____ D E C L A R A T I O N S ________________________________________________

//------------------------------------------------------------------------------
//  @fn DOGS_LCD_init
//!
//! DOGS LCD initialization

extern void DOGS_LCD_init (void);

//------------------------------------------------------------------------------------------------
//  @fn  Dpc
//
//! Write one ASCII character to DOGS graphic display at position Col and Row

extern void Dpc (unsigned char Row, unsigned char Col, unsigned char ASCIIchar);

//------------------------------------------------------------------------------
//  @fn Dpstr
//!
//! Write a string to DOGS graphic display at position Col and Row

extern void Dpstr (unsigned char row, unsigned char col, const char* string);

//------------------------------------------------------------------------------
//  @fn Csr
//!
//! Clear screen at DOGS graphic display

extern void Csr (void);

#endif /* LCD17X8ASCII_H_ */
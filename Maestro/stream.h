/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   stream.h
 * Author: rachad
 *
 * Created on April 13, 2017, 3:47 PM
 */

#ifndef STREAM_H
#define STREAM_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#define DEBUG 0

void init_serial(int fd, struct termios* toptions);
int readline(int fd, char*buffer, int buffersize);
void printline(char* buffer);


#endif /* STREAM_H */


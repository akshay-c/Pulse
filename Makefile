#avr gcc
CC = avr-gcc 
#avr objeccopy
OBJCOPY = avr-objcopy

#The CFLAGS variable sets compile flags for gcc
#-g                 compile with debug information
#-mmcu=atmega328p   selects the target device
#-std=gnu99         use the C99 standard language definition
CFLAGS = -g -mmcu=atmega328 -std=gnu99 -Os

#OBJFLAG sets the flag for making a elf to hex
OBJFLAG = -j .text -j .data -O ihex

EXECUTABLE = virtualglove
SOURCES = ${wildcard *.c}
HEADERS = ${wildcard *.h}
OBJECTS = ${SOURCES:.c=.o}

.PHONY: all
all: ${EXECUTABLE}

$(EXECUTABLE): $(OBJECTS) buildnumber.num
	$(CC) -g -mmcu=atmega328 -o virtualglove.elf $(OBJECTS)
	$(OBJCOPY) $(OBJFLAG) virtualglove.elf virtualglove.hex
	@echo "-- Build: " $$(cat buildnumber.num)

# Creeer dependency file met optie -MM van de compiler
depend: $(SOURCES)
	@echo "calling depend"
	$(CC) $(CFLAGS) -Os -c -MM $^ > $@

-include depend

# Buildnumber administratie
buildnumber.num: $(OBJECTS)
	@if ! test -f buildnumber.num; then echo 0 > buildnumber.num; fi
	@echo $$(($$(cat buildnumber.num)+1)) > buildnumber.num
	
# Create a clean environment
.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)

# Clean up dependency file  
.PHONY: clean-depend
clean-depend: clean
	$(RM) depend  
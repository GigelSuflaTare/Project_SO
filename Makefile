CC = gcc
CCFLAGS = -Wall -g

TARGETS = treasure_manager treasure_hub treasure_monitor

all: $(TARGETS)

treasure_manager: treasure_manager.c
	$(CC) $(CCFLAGS) treasure_manager.c -o treasure_manager

treasure_hub: treasure_hub.c
	$(CC) $(CCFLAGS) treasure_hub.c -o treasure_hub

treasure_monitor: treasure_monitor.c
	$(CC) $(CCFLAGS) treasure_monitor.c -o treasure_monitor

clean:
	rm -f $(TARGETS)


CC=gcc
CFLAGS=-Wall -std=c99 -g -Iinclude
SRCDIR=src
OBJDIR=object

# to use libmessage.so : export LD_LIBRARY_PATH=${PWD} on the shell

all : period periodic launch_daemon


periodic : $(OBJDIR)/periodic.o libmessage.so libperror.so $(OBJDIR)/file.o
	@echo make : $@
	@$(CC) $(CFLAGS) -L${PWD} $< $(OBJDIR)/file.o -lmessage -lperror  -o $@

period : $(OBJDIR)/period.o libmessage.so libperror.so $(OBJDIR)/command.o $(OBJDIR)/file.o
	@echo make : $@
	@$(CC) $(CFLAGS) -L${PWD} $< $(OBJDIR)/command.o $(OBJDIR)/file.o -lmessage -lperror  -o $@


lib%.so : $(OBJDIR)/%.o 
	@echo make : $@
	@$(CC) -shared $< -o $@


$(OBJDIR)/message.o : $(SRCDIR)/message.c
	@echo make : $@
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(OBJDIR)/perror.o : $(SRCDIR)/perror.c
	@echo make : $@
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<


$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo make : $@
	@$(CC) $(CFLAGS) -c -o $@ $<

% : $(SRCDIR)/%.c 
	@echo make : $@
	@$(CC) $(CFLAGS) -o $@ $<


clean : 
	rm -f *.o


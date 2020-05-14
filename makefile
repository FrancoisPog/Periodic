CC=gcc
CFLAGS=-Wall -std=c99 -g -Iinclude
SRCDIR=src
OBJDIR=object

# to use libmessage.so : export LD_LIBRARY_PATH=${PWD} on the shell

all : period periodic test_libcommand test_libmessage  launch_daemon

periodic : $(OBJDIR)/periodic.o libmessage.so libcommand.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -lcommand -o $@

period : $(OBJDIR)/period.o libmessage.so libcommand.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -lcommand -o $@

test_libmessage : $(OBJDIR)/test_libmessage.o libmessage.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -o $@

test_libcommand : $(OBJDIR)/test_libcommand.o libcommand.so
	@echo make : $@
	@gcc -L${PWD} $< -lcommand -o $@
	
lib%.so : $(OBJDIR)/%.o 
	@echo make : $@
	@$(CC) -shared $< -o $@

$(OBJDIR)/command.o : $(SRCDIR)/command.c
	@echo make : $@
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(OBJDIR)/message.o : $(SRCDIR)/message.c
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


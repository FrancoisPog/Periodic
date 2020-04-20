CC=gcc
CFLAGS=-Wall -std=c99 -g -Iinclude
SRCDIR=src
OBJDIR=object

# to use libmessage.so : export LD_LIBRARY_PATH=${PWD} on the shell

periodic : $(OBJDIR)/periodic.o libmessage.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -o $@

period : $(OBJDIR)/period.o libmessage.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -o $@

test_libmessage : $(OBJDIR)/test_libmessage.o libmessage.so
	@echo make : $@
	@gcc -L${PWD} $< -lmessage -o $@
	
libmessage.so : $(OBJDIR)/message.o 
	@echo make : $@
	@$(CC) -shared $< -o $@

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


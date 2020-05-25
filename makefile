CC=gcc
CFLAGS=-Wall -std=c99 -g -Iinclude
SRCDIR=src
OBJDIR=object
PROGRAMS = period periodic launch_daemon
LIBS = libmessage.so libperror.so
OTHER = now when

# to use libmessage.so : export LD_LIBRARY_PATH=${PWD} on the shell

.PHONY : all
all : $(PROGRAMS) $(LIBS) $(OTHER)

.PHONY : prog
prog : $(PROGRAMS)

.PHONY : lib
lib : $(LIBS)

.PHONY : other
other : $(OTHER)


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


.PHONY : clean
clean : 
	rm -f $(OBJDIR)/*.o


.PHONY : mrproper
mrproper : clean
	rm -f $(PROGRAMS) $(LIBS) $(OTHER)
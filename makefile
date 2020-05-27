# Compilation
CC=gcc
CFLAGS=-Wall -std=c99 -g -Iinclude

# Directory
INCDIR=include
SRCDIR=src
OBJDIR=object

# Main rules
PROGRAMS = period periodic launch_daemon
LIBS = libmessage.so libperror.so
OTHER = now when

# Macro
OBJDIR_CREATE = if ! test -d $(OBJDIR) ; then mkdir $(OBJDIR) ; fi
OBJDIR_DELETE = if test -d $(OBJDIR) ; then rmdir $(OBJDIR) ; fi


# to use libmessage.so : export LD_LIBRARY_PATH=${PWD} on the shell

.PHONY : all
all : $(LIBS) $(PROGRAMS)  $(OTHER)

.PHONY : prog
prog : lib $(PROGRAMS)

.PHONY : lib
lib : $(LIBS)

.PHONY : other
other : $(OTHER)

launch_daemon : $(OBJDIR)/launch_daemon.o $(INCDIR)/perror.h
	@echo make : launch_daemon
	@$(CC) $(CFLAGS) -L${PWD} $< -lperror  -o $@


periodic : $(OBJDIR)/periodic.o   $(OBJDIR)/file.o $(INCDIR)/perror.h $(INCDIR)/message.h
	@echo make : $@
	@$(CC) $(CFLAGS) -L${PWD} $< $(OBJDIR)/file.o -lmessage -lperror  -o $@


period : $(OBJDIR)/period.o  $(OBJDIR)/command.o $(OBJDIR)/file.o $(INCDIR)/perror.h $(INCDIR)/message.h
	@echo make : $@
	@$(CC) $(CFLAGS) -L${PWD} $< $(OBJDIR)/command.o $(OBJDIR)/file.o -lmessage -lperror  -o $@


lib%.so : $(OBJDIR)/%.o 
	@echo make : $@
	@$(CC) -shared $< -o $@


$(OBJDIR)/message.o : $(SRCDIR)/message.c $(INCDIR)/message.h
	@echo make : $@
	@$(OBJDIR_CREATE)
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<


$(OBJDIR)/perror.o : $(SRCDIR)/perror.c $(INCDIR)/perror.h
	@echo make : $@
	@$(OBJDIR_CREATE)
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<


$(OBJDIR)/%.o : $(SRCDIR)/%.c 
	@echo make : $@
	@$(OBJDIR_CREATE)
	@$(CC) $(CFLAGS) -c -o $@ $<


% : $(SRCDIR)/%.c
	@echo make : $@
	@$(CC) $(CFLAGS) -o $@ $<



.PHONY : clean
clean : 
	@echo make : clean
	@rm -f $(OBJDIR)/*.o ;
	@$(OBJDIR_DELETE)


.PHONY : mrproper
mrproper : clean
	@echo make : mrproper
	@rm -f $(PROGRAMS) $(LIBS) $(OTHER)
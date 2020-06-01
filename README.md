# Periodic

***[ University Project | Spring 2020 ]***

Program to execute commands at regular time intervals. C programming project. 

## How to use 
- Download `period`, `periodic` and `launch_daemon`
- Launch `period` as a daemon with `launch_damon`
- Use `periodic` to interact with `period`

## Features
- Launch period : 
```bash
./launch_daemon ${PWD}/period
```
- Add a command : 
```bash
./periodic start period command [args...]
```
- Get the command list : 
```bash
./periodic
```
- Remove a command from list : 
```bash
./periodic remove command_id
```

## Operating system learning
- Process
- Pipes
- Signals

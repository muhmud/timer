#!/bin/bash

TIMER=~/.timer/current

if [[ -f $TIMER ]]; then
  TIMER_DIRECTORY=$(cat $TIMER)
  TIMER_STATUS=$(timer --status -D "$TIMER_DIRECTORY")
  STATUS=$(echo $TIMER_STATUS | jq --raw-output '.timer.current.status')
  
  if [[ "$STATUS" != "STOPPED" ]]; then
    if [[ "$STATUS" == "RUNNING" ]]; then
      ~/bin/timer --pause -D "$TIMER_DIRECTORY"
    else
      ~/bin/timer --resume -D "$TIMER_DIRECTORY"
    fi
  else
    TASK_DESCRIPTION=$(zenity --entry --width=500 --title="Start Timer (`basename $TIMER_DIRECTORY`)" --text="Enter Task Description:" --ok-label="Start")
    if [[ ! -z $TASK_DESCRIPTION ]]; then
      ~/bin/timer --go -T "$TASK_DESCRIPTION" -D "$TIMER_DIRECTORY"
    fi
  fi
fi

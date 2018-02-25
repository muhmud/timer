#!/bin/bash

TIMER=~/.timer/current

if [[ -f $TIMER ]]; then
  TIMER_DIRECTORY=$(cat $TIMER)
  TIMER_STATUS=$(timer --status -D "$TIMER_DIRECTORY")
  STATUS=$(echo $TIMER_STATUS | jq --raw-output '.timer.current.status')

  if [[ "$STATUS" == "PAUSED" ]]; then
    TASK_DESCRIPTION=$(zenity --entry --width=500 --title="Change Task (`basename $TIMER_DIRECTORY`)" --text="Enter New Task Description:" --ok-label="Start")
    if [[ ! -z $TASK_DESCRIPTION ]]; then
      ~/bin/timer --resume -T "$TASK_DESCRIPTION" -D "$TIMER_DIRECTORY"
    fi
  fi
fi

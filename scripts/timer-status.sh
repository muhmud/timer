#!/bin/bash

TIMER=~/.timer/current

if [[ -f $TIMER ]]; then
  TIMER_DIRECTORY=$(cat $TIMER)
  TIMER_STATUS=$(timer --status -D "$TIMER_DIRECTORY")
  PROJECT=$(echo $TIMER_STATUS | jq --raw-output '.project')
  STATUS=$(echo $TIMER_STATUS | jq --raw-output '.timer.current.status')
  WORK_DONE=$(echo $TIMER_STATUS | jq --raw-output '.timer.workDone')

  if [[ "$STATUS" == "RUNNING" ]]; then
      echo " $PROJECT - %{F#f00}$WORK_DONE%{F-}"
  elif [[ "$STATUS" == "PAUSED" ]]; then
      echo "$PROJECT - %{F#0f0}$WORK_DONE%{F-}"
  else
      echo "";
  fi
else
    echo "";
fi

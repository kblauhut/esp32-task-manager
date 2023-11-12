"use client";

import React, { useEffect, useState, useRef } from "react";
import { ResponsiveStream } from "@nivo/stream";
import { ResponsivePie } from "@nivo/pie";

// UI
import { Circle, Center, Grid, Stack, styled } from "panda-css/jsx";

type TaskInfo = {
  type: "task_info";
  total_run_time: number;
  tasks: {
    name: string;
    priority: number;
    task_number: number;
    state: number;
    stack: number;
    runtime: number;
  }[];
};

export const EspDashboard = () => {
  const socketRef = useRef<WebSocket>();
  const [taskInfoData, setTaskInfoData] = useState<TaskInfo[]>([]);
  const [taskActivity, setTaskActivity] = useState<Record<string, number>[]>(
    []
  );

  useEffect(() => {
    const socket = new WebSocket("ws://localhost:3010");
    socketRef.current = socket;

    socket.addEventListener("message", async (event) => {
      if (!(event.data instanceof Blob)) return;
      const json = JSON.parse(await event.data.text());
      setTaskInfoData((prev) => [...prev, json]);
    });

    return () => {
      socket.close();
    };
  }, []);

  useEffect(() => {
    const currentTaskInfo = taskInfoData[taskInfoData.length - 1];
    const prevTaskInfo = taskInfoData[taskInfoData.length - 2];
    if (!prevTaskInfo) return;

    const activity: Record<string, number> = {};
    currentTaskInfo.tasks.forEach((task) => {
      if (task.name === "IDLE") return;
      const prevTask = prevTaskInfo.tasks.find(
        (t) => t.task_number === task.task_number
      );
      if (!prevTask) return;
      const taskRunTime = task.runtime - prevTask.runtime;

      activity[task.name] = taskRunTime;
    });
    setTaskActivity((prev) => [...prev, activity]);
  }, [taskInfoData]);

  return (
    <Grid h="800px" gridTemplateRows="1fr 1fr" gridTemplateColumns="2fr 1fr">
      <styled.div bg="white" borderRadius="8px">
        <ResponsiveStream
          keys={Object.keys(taskActivity[taskActivity.length - 1] || {})}
          data={taskActivity}
          margin={{ top: 100, right: 100, bottom: 100, left: 100 }}
          animate={false}
          axisBottom={null}
          axisLeft={{
            legendOffset: -1112,
          }}
        />
      </styled.div>
      <styled.div bg="white" borderRadius="8px">
        {taskInfoData[taskInfoData.length - 1] && (
          <ResponsivePie
            data={taskInfoData[taskInfoData.length - 1].tasks.flatMap(
              (task) => {
                if (task.name === "IDLE") return [];
                return {
                  id: task.name,
                  label: task.name,
                  value: task.stack,
                };
              }
            )}
            margin={{ top: 60, right: 60, bottom: 60, left: 60 }}
            animate={false}
          />
        )}
      </styled.div>
      <Stack
        bg="white"
        borderRadius="8px"
        cursor="pointer"
        overflow="scroll"
        p="12px"
      >
        {taskInfoData[taskInfoData.length - 1] &&
          taskInfoData[taskInfoData.length - 1].tasks.map((task) => (
            <Grid gridTemplateColumns="2fr 1fr 1fr" key={task.task_number}>
              <styled.div>{task.name}</styled.div>

              <styled.div>{task.stack}</styled.div>
              <styled.div>{task.runtime}</styled.div>
            </Grid>
          ))}
      </Stack>

      <Center bg="white" borderRadius="8px" cursor="pointer">
        <Circle
          size="130px"
          bg="red.700"
          boxShadow="0px 8px 0px 0px #f54242"
          color="white"
          fontSize="20px"
          fontWeight="bold"
          textAlign="center"
          _active={{
            boxShadow: "0px 0px 0px 0px #f54242",
            transform: "translateY(8px)",
          }}
          onClick={() => {
            socketRef.current?.send("add_thread");
          }}
        >
          ADD THREAD
        </Circle>
      </Center>
    </Grid>
  );
};

import React from "react";

// UI
import { Stack, styled } from "panda-css/jsx";

// Components
import { EspDashboard } from "./_client/EspDashboard";

export default async function Page() {
  return (
    <Stack m="16px" overflow="scroll">
      <styled.h1 fontWeight="500" fontSize="28px">
        ESP Dashboard
      </styled.h1>
      <EspDashboard />
    </Stack>
  );
}

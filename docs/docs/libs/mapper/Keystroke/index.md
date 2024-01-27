---
sidebar_position: 1
id: Keystroke_index
---

# Keystroke object
Keystroke object represents a keystroke sequense to emulate keyboard.<br/>
The Keystroke object, contrary to its name, does not represent a simple key press or release; instead, it can express a sequence of continuous key inputs.

## Constructors
|Constructor|
|---|
|[`mapper.keystroke()`](/libs/mapper/mapper_keystroke)

## Properties
|Name|Description|
|-|-|
|[```Keystroke.duration```](/libs/mapper/Keystroke/Keystroke_duration)|The duration of key press|
|[```Keystroke.interval```](/libs/mapper/Keystroke/Keystroke_interval)|The interval between keystrokes|

## Methods
|Name|Description|
|-|-|
|[```Keystroke:synthesize()```](/libs/mapper/Keystroke/Keystroke-synthesize)|Synthesize keystroke sequense held by this object|
|[```Keystroke:synthesizer()```](/libs/mapper/Keystroke/Keystroke-synthesizer)|Create a  native-action to synthesize keystroke sequense hold by this object|

## Operators
|Operator|Description|
|-|-|
|[`+`](/libs/mapper/Keystroke/Metatable_add)|Concatenating operation for keystrokes
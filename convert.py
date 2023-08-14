#!/usr/bin/env python3

import sys
import re

class SyntaxError(Exception):
  def __init__(self,text,statements=""):
    self.text=text
    self.line=None
    self.statements=statements
  def setLine(self,line):
    self.line=line
  def __str__(self):
    return "Error on line "+str(self.line)+": "+self.text+" Statements: "+str(self.statements)

class Output(object):
  def __init__(self,out=sys.stdout):
    self.out=out
    self.currentOffset=0
    self.currentAdd=0
    self.whileOffsets=[]
    self.shouldPrintNewline=0
  def _printAdd(self):
    "Print the + and - for the current position. Print nothing when we don't need to"
    print( ('+' if self.currentAdd>0 else '-')*abs(self.currentAdd), end='', file=self.out )
    if self.currentAdd :
      print("\n"*self.shouldPrintNewline, end='', file=self.out)
      self.shouldPrintNewline=0
    self.currentAdd=0
  def _printOffset(self):
    "Print the > and < for the current position. Print nothing when we don't need to"
    print( ('>' if self.currentOffset>0 else '<')*abs(self.currentOffset), end='', file=self.out )
    if self.currentOffset :
      print("\n"*self.shouldPrintNewline, end='', file=self.out)
      self.shouldPrintNewline=0
    self.currentOffset=0
  def goto(self,offset):
    if offset:
      self._printAdd()
    self.currentOffset = self.currentOffset + offset
  def add(self,n):
    if n:
      self._printOffset()
    self.currentAdd = self.currentAdd + n
  def flush(self):
    self._printOffset()
    self._printAdd()
  def command(self,command):
    self.flush()
    print(command, end='', file=self.out)
  def nextLine(self):
    self.shouldPrintNewline = self.shouldPrintNewline+1

  def commandOffset(self,command,offset):
    self.goto(offset)
    self.command(command)
    self.goto(-offset)
  def startWhile(self,offset):
    self.commandOffset('[',offset)
    self.whileOffsets.append(offset)
  def endWhile(self):
    offset=self.whileOffsets.pop()
    self.commandOffset(']',offset)
  def setValue(self,offset,value):
    command = '[-]'+('+' if value>0 else '-')*abs(value)
    self.commandOffset(command,offset)
  def copyValue(self,sourceOffset,targetOffset,temporaryOffset):
    self.commandOffset('[-]',targetOffset)

    self.commandOffset('[-',sourceOffset)
    self.commandOffset('+',temporaryOffset)
    self.commandOffset('+',targetOffset)
    self.commandOffset(']',sourceOffset)

    self.commandOffset('[-',temporaryOffset)
    self.commandOffset('+',sourceOffset)
    self.commandOffset(']',temporaryOffset)

  def addOffset(self,offset,value):
    self.goto(offset)
    self.add(value)
    self.goto(-offset)


def getAccessOffset(statements):
  if len(statements)<4:
    raise SyntaxError("Expected data[<something>] but there are too few statements",statements)
  if statements[0]!='data':
    raise SyntaxError("expected data [<something] got ",statements[0],statements)
  if statements[1]!='[':
    raise SyntaxError("expected [ got ",statements[1],statements)

  statements=statements[2:]

  if len(statements)<2:
    raise SyntaxError("data[...] access without enoug elements",statements)
  if statements[0]!='p':
    raise SyntaxError("data[...] access that doesn't start with a p",statements)
  statements=statements[1:]

  n=0
  while 1:
    if len(statements)<1 or statements[0]==']':
      return n,statements[1:]
    if statements[0]=='+':
      n=n+int(statements[1])
      statements=statements[2:]
    elif statements[0]=='-':
      n=n-int(statements[1])
      statements=statements[2:]
    else:
      raise SyntaxError("data[...] with unknown statement"+str(statements[0]),statements)



output=Output()



def handleWriteChar(statements):
  if len(statements)<4 or statements[1]!='(' or statements[-1]!=')':
    raise SyntaxError("Unexpected data for write_char",statements)

  offset,statements = getAccessOffset(statements[2:-1])
  if len(statements):
    raise SyntaxError("Found additional data after write_char ( data[...] ) ",statements)
  output.commandOffset('.',offset)

def handleWhile(statements):
  if len(statements)<4:
    raise SyntaxError("Found { without a proper while statement",statements)
  if statements[0]!='while':
    raise SyntaxError("Found { without while",statements)
  if statements[1]!='(':
    raise SyntaxError("Found while without (",statements)
  if statements[-1]!=')':
    raise SyntaxError("Found while without )",statements)

  offset,statements = getAccessOffset(statements[2:-1])
  if len(statements):
    raise SyntaxError("Found additional data after while (...) ",statements)
  output.startWhile(offset)

def endWhile(statements):
  if len(statements)>1:
    raise SyntaxError("Found } with extra, unexpected data",statements)
  output.endWhile()


def handleAssignment(statements):
  if len(statements)<2:
    raise SyntaxError("not enough elements for assignment",statements)

  setP=0
  targetOffset=None
  if statements[0]=='data':
    targetOffset,statements = getAccessOffset(statements)
  elif statements[0]=='p':
    setP=1
    statements = statements[1:]
    if len(statements)==2:
      if statements[0]=='+' and statements[1]=='+':
        output.add(1)
        return
      if statements[0]=='-' and statements[1]=='-':
        output.add(-1)
        return
  elif statements[0]=='write_char':
    handleWriteChar(statements)
    return
  else:
    raise SyntaxError("Unknwon how to handle "+statements[0],statements)


  add=0
  startOffset=None
  if len(statements)<2:
    raise SyntaxError("not enough elements for assignment",statements)
  if statements[0]!='=':
    if len(statements)==2 and statements[0]=='-' and statements[1]=='-':
      startOffset=targetOffset
      add=-1
    elif len(statements)==2 and statements[0]=='+' and statements[1]=='+':
      startOffset=targetOffset
      add=+1
    else:
      raise SyntaxError("expected =, ++ or -- got "+statements[0],statements)
    statements=[]
  else:
    statements=statements[1:]


    if statements[0]=='data':
      if setP:
        raise SyntaxError("can not set p from data",statements)
      startOffset,statements = getAccessOffset(statements)

    elif statements[0]=='p':
      if not setP:
        raise SyntaxError("can not set data from p",statements)
      statements=statements[1:]


  isReadChar=0
  while len(statements):
    if statements[0] in '0123456789':
      if add or isReadChar:
        raise SyntaxError("Unexpected integer",statements)
      add = int(statements[0])
      statements=statements[1:]

    elif statements[0]=='+' or statements[0]=='-':
      isAddition = statements[0]=='+'
      n=0
      if len(statements)<2:
        raise SyntaxError("Expected something after +",statements)
      if statements[1]=="'":
        if len(statements)<3:
          raise SyntaxError("Not enough elements for character literal",statements)
        if statements[3]!="'":
          raise SyntaxError("No valid character literal",statements)
        n = ord(statements[2])
        statements=statements[4:]
      else:
        n = int(statements[1])
        statements=statements[2:]
      if isAddition:
        add = add + n
      else:
        add = add - n

    elif statements[0]=='read_char':
      if len(statements)<3:
        raise SyntaxError("Expected () after read_char",statements)
      if statements[1]!="(" or statements[2]!=')' :
        raise SyntaxError("Expected () after read_char",statements)
      isReadChar=1
      statements=statements[3:]
    else:
      raise SyntaxError("expected + or - got "+statements[0],statements)


  if setP:
    if targetOffset is not None or startOffset is not None or isReadChar:
      raise SyntaxError("can not set p from data"+statements[0],statements)
    output.goto(add)
    return

  if targetOffset is None:
    raise SyntaxError("Unexpected assignment format",statements)

  if isReadChar:
    if startOffset is not None:
      raise SyntaxError("can not add read_char() with data ",statements)
    output.commandOffset(',',targetOffset)
    return


  if startOffset is None:
    output.setValue(targetOffset,add)
    return

  if startOffset == targetOffset:
    output.addOffset(targetOffset,add)
    return

  #Copy a cell to target cell and to cell with offset 7
  #After that we copy cell 7 to source cell
  #And after that we may need to add/substract a fixed value
  output.copyValue(startOffset,targetOffset,7)
  if add:
    output.addOffset(targetOffset,add)




def addElementStatement(statement,element):
  element=element.strip()
  if len(element)==0:
    return statement
  if '{' in e:
    handleWhile(statement)
    statement=[]
  elif '}' in e:
    endWhile(statement)
    statement=[]
  elif ';' in e:
    handleAssignment(statement)
    statement=[]
  else:
    statement.append(element)
  return statement



statement=[]
for line,i in enumerate(sys.stdin):
  try:
    #for e in i.split():
    for e in re.split('([^a-zA-Z0-9_])',i):
      line=0
      if '\n' in e:
        line =1
      statement = addElementStatement(statement,e)
      if line: output.nextLine()
  except SyntaxError as e:
    e.setLine(line+1)
    raise

try:
  if len(addElementStatement(statement,e)):
    raise SyntaxError("Extra data at the end of the file")
except SyntaxError as e:
  e.setLine(line)
  raise

output.flush()



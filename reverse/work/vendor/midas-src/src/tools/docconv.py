#*      docconv.py
#*
#* Converts MIDAS m4/LaTeX documents to WinHelp
#*
#* $Id: docconv.py,v 1.6 1997/08/15 13:18:54 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


import sys
import string
import regex


class WinhelpWriter:
    def __init__(self, fileName, quiet=0):
        self.quiet = quiet
        self.seqCounter = 1
        self.prevSectionLevel = 1

        self.inParagraph = 0
        self.startParagraph = 1
        
        self.rtff = 0;
        self.cntf = open(fileName + ".cnt", "w")
        self.hpjf = open(fileName + ".hpj", "w")

        self.hpjf.write("[OPTIONS]\n"
                        "HCW=0\n"
                        "LCID=0x409 0x0 0x0 ;English (United States)"
                        "REPORT=Yes\n"
                        "FTS=1\n"
                        "TITLE=MIDAS Digital Audio System\n"
                        "CNT=" + fileName + ".cnt \n"
                        "HLP=" + fileName + ".hlp\n"
                        "COPYRIGHT=This document is a part of MIDAS Digital "
                        "Audio System, Copyright 1997 Housemarque Inc.\n"
                        "\n"
                        "[FILES]\n")

        self.cntf.write(":Base " + fileName + "\n"
                        ":Title MIDAS Digital Audio System\n")

    def __del__(self):
        self.hpjf.write("\n"
                        "[CONFIG]\n"
                        "BrowseButtons()\n")
        if self.rtff:
            self.rtff.write("\page\n}\n")            

    def NewFile(self, fileName):
        self.hpjf.write(fileName + ".rtf\n")
        if self.rtff:
            self.rtff.write("\page\n}\n")
            self.rtff.close
        self.rtff = open(fileName + ".rtf", "w")

        self.rtff.write("{\\rtf\\ansi\n\n"
                        "{\\fonttbl\\f0\\froman Times New Roman;"
                        "\\f1\\fmodern Courier New;"
                        "\\f2\\fswiss Arial;}"
                        "\\deff2\\sa200\n\n")

    def ScanFile(self):
        self.sections = []
        self.numSections = 0
        self.sectionNumber = 0
        if not self.quiet:
            print "Scanning..."


    def ScanSection(self, level):
        self.sections.append((level, 0))
        self.numSections = self.numSections + 1

    def ScanText(self):
        if self.numSections:
            self.sections[self.numSections-1] = (self.sections[
                self.numSections-1][0], 1)
        

    def NewSection(self, level, title, name=0, keywords=""):
        if not self.quiet:
            sys.stdout.write("Converting section %i/%i\r" %
                             (self.sectionNumber,self.numSections-1))
            sys.stdout.flush()
            if self.sectionNumber == (self.numSections-1):
                print ""
        
        hasSub = 0
        hasText = 0
        if self.sectionNumber < (self.numSections-1):
            if self.sections[self.sectionNumber+1][0] > level:
                hasSub = 1
        if self.sections[self.sectionNumber][1]:
            hasText = 1

        self.sectionNumber = self.sectionNumber + 1

        if hasSub:
            self.cntf.write(str(level) + " " + title + "\n")

        if hasText:
            self.cntf.write(str(level) + " " + title + "=" + name + "\n")
            
            self.rtff.write("\n\page\n")
            self.rtff.write("#{\\footnote " + name + "}\n"
                            "${\\footnote " + title + "}\n")

            if keywords != "":
                self.rtff.write            
                self.rtff.write("K{\\footnote ")
                if (keywords[0] == 'k') or (keywords[0] == 'K'):
                    self.rtff.write(" ")
                self.rtff.write(keywords + "}\n")
            self.rtff.write("+{\\footnote lahna: %08i}\n\n" % self.seqCounter)
            self.seqCounter = self.seqCounter + 1

            self.rtff.write("\\keepn\\sb100\\li480\n"
                            "{\\f2\\fs24\\b " + title + "}\n"
                            "\\sa100\\par\\pard\n\\fs20\\sb100\\li480\n\n")

            self.inParagraph = 0
            self.startParagraph = 1

            
    def ParagraphBreak(self):
        self.rtff.write("\n")
        self.inParagraph = 0


    def FixString(self, s):
        s = string.joinfields(string.splitfields(string.joinfields(
            string.splitfields(string.joinfields(
                string.splitfields(s, '\\'), '\\\\'), '{'), '\\{'), '}'),
                              '\\}')
        return s


    def PrepareParagraph(self):
        if not self.inParagraph:
            if not self.startParagraph:
                self.rtff.write("\n\\sa200\\par\\pard\n")
            self.rtff.write("\\li480\n")
        self.inParagraph = 1
        self.startParagraph = 0

        
    def ParagraphText(self, text):
        self.PrepareParagraph()        
        self.rtff.write(self.FixString(text))


    def LeftDoubleQuote(self):
        self.PrepareParagraph()
        self.rtff.write("\\ldblquote ")


    def RightDoubleQuote(self):
        self.PrepareParagraph()
        self.rtff.write("\\rdblquote ")


    def EmDash(self):
        self.PrepareParagraph()
        self.rtff.write("\\emdash ")

       
    def BeginVerbatim(self):
        if not self.startParagraph:
            self.rtff.write("\n\\sa200\\par\\pard\n")
        self.rtff.write("\\li480\n{\\f1\\fs18\n")
        self.inParagraph = 1
        self.startParagraph = 0
        self.hadVerbatimLine = 0

    def EndVerbatim(self):
        self.rtff.write("\n}\n")
        self.inParagraph = 0
        self.startParagraph = 0

        
    def VerbatimLine(self, text):
        if self.hadVerbatimLine:
            self.rtff.write("\\line\n")
        self.rtff.write(self.FixString(text))
        self.hadVerbatimLine = 1

        
    def ParagraphTitle(self, title):
        if not self.startParagraph:
            self.rtff.write("\n\\sa200\\par\\pard")
        self.rtff.write("{\\li0\\b\\fs22 " + title + "}\n")
        self.rtff.write("\\par\\pard")
        self.inParagraph = 0
        self.startParagraph = 1

    
    def BeginDescriptionList(self):
        self.rtff.write("\n")

        
    def EndDescriptionList(self):
        self.startParagraph = 0
        self.inParagraph = 0
    
        
    def DescriptionListItem(self, item):
        if not self.startParagraph:
            self.rtff.write("\n\\par\\pard")
        self.rtff.write("\\li480 {\\b " + item + "}\n"
                        "\\par\\pard\\sa100\\li960 ")
        self.inParagraph = 1
    

    def BeginBoldText(self):
        self.rtff.write("{\\b ")

    def EndBoldText(self):
        self.rtff.write("}")


    def BeginItalicsText(self):
        self.rtff.write("{\\i ")

    def EndItalicsText(self):
        self.rtff.write("}")


    def BeginEmphasizedText(self):
        self.rtff.write("{\\i ")

    def EndEmphasizedText(self):
        self.rtff.write("}")

        
    def BeginTeletype(self):
        self.rtff.write("{\\f1\\fs18 ")

    def EndTeletype(self):
        self.rtff.write("}")


    def LinkText(self, target, text):
        self.PrepareParagraph()
        self.rtff.write("{\\uldb " + text + "}{\\v " + target + "}")
        


class PlainTextWriter:
    def __init__(self, fileName, quiet=0):
        self.quiet = quiet
        self.seqCounter = 1
        self.prevSectionLevel = 1
        self.numSections = 0

        self.inParagraph = 0
        self.startParagraph = 1

        self.lineWidth = 78
        self.normalTextIndent = 4
        self.textIndent = 4
        self.titleIndent = 0

        self.f = 0
        self.line = ""
        self.textIndentString = self.DupChar(" ", self.textIndent)

        self.maxSectLevel = 3

        self.sectNums = range(self.maxSectLevel)
        for i in range(self.maxSectLevel):
            self.sectNums[i] = 0

    def __del__(self):
        self.ParagraphBreak()

    def NewFile(self, fileName):
        if self.f:
            self.ParagraphBreak()
            self.f.close
        self.f = open(fileName + ".txt", "w")

    def ScanFile(self):
        self.sections = []
        self.numSections = 0
        self.sectionNumber = 0
        if not self.quiet:
            print "Scanning..."

    def ScanSection(self, level):
        self.sections.append((level, 0))
        self.numSections = self.numSections + 1

    def ScanText(self):
        if self.numSections:
            self.sections[self.numSections-1] = (self.sections[
                self.numSections-1][0], 1)

            
    def DupChar(self, char, num):
        s = ""
        for i in range(num):
            s = s + char
        return s
        

    def NewSection(self, level, title, name=0, keywords=""):
        if not self.quiet:
            sys.stdout.write("Converting section %i/%i\r" %
                             (self.sectionNumber,self.numSections-1))
            sys.stdout.flush()
            if self.sectionNumber == (self.numSections-1):
                print ""

        self.ParagraphBreak()

        hasText = 0
        if self.sections[self.sectionNumber][1]:
            hasText = 1
            
        if level == 1:
            s = self.DupChar(" ", (self.lineWidth - len(title)) / 2);
            self.f.write(s + title + "\n")
            self.f.write(s + self.DupChar("=", len(title)) + "\n\n")
                        
        elif level == 2:
            self.f.write("\n\n")
            if self.sectNums[0] > 0:
                self.f.write(self.DupChar("-", self.lineWidth) + "\n\n")
            self.sectNums[0] = self.sectNums[0] + 1;
            for i in range(1, self.maxSectLevel) :
                self.sectNums[i] = 0;
            line = str(self.sectNums[0]) + ". " + title
            s = self.DupChar(" ", (self.lineWidth - len(line)) / 2);
            self.f.write(s + line + "\n")
            if hasText:
                self.f.write("\n\n")            
#            self.f.write(s + self.DupChar("=", len(line)))

        elif level == 3:
            self.f.write("\n")
            self.sectNums[1] = self.sectNums[1] + 1;
            for i in range(2, self.maxSectLevel) :
                self.sectNums[i] = 0;
            line = str(self.sectNums[0]) + "." + str(self.sectNums[1]) + ". "
            line = line + title
            s = self.DupChar("-", len(line))
            ss = self.DupChar(" ", self.titleIndent)
            self.f.write(ss + s + "\n" + ss + line + "\n" + ss + s + "\n")
            if hasText:
                self.f.write("\n");

        elif level == 4:
            self.f.write("\n")
            self.sectNums[2] = self.sectNums[2] + 1;
            for i in range(3, self.maxSectLevel) :
                self.sectNums[i] = 0;
            line = str(self.sectNums[0]) + "." + str(self.sectNums[1]) + "."
            line = line + str(self.sectNums[2]) + ". " + title
            s = self.DupChar("-", len(line))
            ss = self.DupChar(" ", self.titleIndent)
            self.f.write(ss + line + "\n" + ss + s + "\n")
            if hasText:
                self.f.write("\n");
            
        self.sectionNumber = self.sectionNumber + 1

        self.inParagraph = 0
        self.startParagraph = 1

            
    def ParagraphBreak(self):
        if self.line:
            self.f.write(self.textIndentString + self.line + "\n\n")
        self.line = ""
        self.inParagraph = 0


    def FixString(self, s):
        return s


    def PrepareParagraph(self):
        if not self.inParagraph:
            self.line == ""
        self.inParagraph = 1
        self.startParagraph = 0

        
    def ParagraphText(self, text):
        self.PrepareParagraph()
        line = self.line + text
        while len(line) > (self.lineWidth - self.textIndent):
            l = len(line)
            p = 0
            while p < (self.lineWidth - self.textIndent):
                if line[p] == ' ':
                    l = p;
                p = p + 1;
            
            self.f.write(self.textIndentString + string.strip(line[:l]) + "\n")
            line = line[(l+1):]
        self.line = line                         


    def LeftDoubleQuote(self):
        self.ParagraphText("\"")

    def RightDoubleQuote(self):
        self.ParagraphText("\"")

    def EmDash(self):
        self.ParagraphText("--")
       
    def BeginVerbatim(self):
        self.ParagraphBreak()
        self.inParagraph = 1
        self.startParagraph = 0

    def EndVerbatim(self):
        self.f.write("\n")
        self.inParagraph = 0
        self.startParagraph = 0
       
    def VerbatimLine(self, text):
        self.f.write(self.textIndentString + self.FixString(text) + "\n")

        
    def ParagraphTitle(self, title):
        self.ParagraphBreak()
        ss = self.DupChar(" ", self.titleIndent)
        self.f.write(ss + title + "\n")
        self.inParagraph = 0
        self.startParagraph = 1

    
    def BeginDescriptionList(self):
        self.ParagraphBreak()

        
    def EndDescriptionList(self):
        self.ParagraphBreak()
        self.textIndent = self.normalTextIndent
        self.textIndentString = self.DupChar(" ", self.textIndent)
        self.startParagraph = 0
        self.inParagraph = 0
    
        
    def DescriptionListItem(self, item):
        self.ParagraphBreak()
        self.f.write(self.DupChar(" ", self.normalTextIndent) + item + "\n")
        self.line = ""
        self.textIndent = self.normalTextIndent + 8
        self.textIndentString = self.DupChar(" ", self.textIndent)
        self.inParagraph = 1
    

    def BeginBoldText(self):
        a = 0
    def EndBoldText(self):
        a = 0
    def BeginItalicsText(self):
        a = 0
    def EndItalicsText(self):
        a = 0
    def BeginEmphasizedText(self):
        a = 0
    def EndEmphasizedText(self):
        a = 0
    def BeginTeletype(self):
        a = 0
    def EndTeletype(self):
        a = 0

    def LinkText(self, target, text):
        self.ParagraphText(text)
        
    
    

    
class MdocConverter:

    def __init__(self, writer):
        self.writer = writer

        self.chapterRegex = regex.compile("^\\\\chapter{\([^}]+\)}")
        self.titleRegex = regex.compile("^\\\\title{\([^}]+\)}")
        self.sectionRegex = regex.compile("^\\\\section{\([^}]+\)}")
        self.subsectionRegex = regex.compile("^\\\\subsection{\([^}]+\)}")
        self.paragraphTitleRegex = regex.compile(
            "^\\\\subsubsection\*{\([^}]+\)}")
        self.mDocEntryRegex = regex.compile("^\\\\mDocEntry{\([^}]+\)}")
        self.beginRegex = regex.compile("^\\\\begin{\([^}]+\)}")
        self.endRegex = regex.compile("^\\\\end{\([^}]+\)}")
        self.itemRegex = regex.compile("^\\\\item *\[\([^]]+\)\] *")

        self.commandRegex = regex.compile("^\\\\\([a-zA-Z]+\){\([^}]+\)}")
        self.command2Regex = regex.compile("^\\\\\([a-zA-Z]+\){\([^}]+\)}"
                                           "{\([^}]+\)}")

        self.sectionNumber = 0


    def ConvertFile(self, fileName):
        self.inf = open(fileName, "r")
        self.ScanFile()
        
        self.inf = open(fileName, "r")
        self.inDocument = 0
        self.inVerbatim = 0
        self.inSection = 0
        self.inBold = 0
        self.inItalics = 0
        self.inEmphasis = 0

        line = self.inf.readline()

        while line:
            while (len(line) > 0) and (line[-1] == '\012'):
                line = line[:-1]
            
            self.sourceLine = line
            if not self.inVerbatim:
                line = string.strip(line)
            
            if (len(line) == 0) or (line[0] != '%'):
                line = self.StripComments(line)
                if len(line) == 0:
                    self.BlankLine()
                else:
                    self.ConvertLine(line)
            line = self.inf.readline()


    def ScanFile(self):
        self.inDocument = 0
        self.inSection = 0
        self.writer.ScanFile()
        for line in self.inf.readlines():
            line = string.strip(line)
            if (len(line) > 0) and (line[0] != '%'):
                line = self.StripComments(line)
                if len(line) > 0:
                    self.ScanLine(line)
                

    def ScanLine(self, line):
        if self.inDocument:
            if line[0] == '\\':
                if self.chapterRegex.match(line) > 0:
                    self.writer.ScanSection(2)
                    self.inSection = 1
                    return
                elif self.sectionRegex.match(line) > 0:
                    self.writer.ScanSection(3)
                    self.inSection = 1
                    return
                elif self.subsectionRegex.match(line) > 0:
                    self.writer.ScanSection(4)
                    self.inSection = 1
                    return
                elif self.mDocEntryRegex.match(line) > 0:
                    self.writer.ScanSection(4)
                    self.inSection = 1
                    return
                else:
                    if self.inSection:
                        self.writer.ScanText()
                    return
            else:
                if self.inSection:
                    self.writer.ScanText()
                return
        else:
            if self.titleRegex.match(line) > 0:
                self.writer.ScanSection(1)
                return
            if line == "\\begin{document}":
                self.inDocument = 1
        


    def StripComments(self, line):
        if (len(line) == 0) or (line[0] == '%'):
            return ""
        cstart = string.find(line, "%")
        while (cstart > 0) and (line[cstart-1] == '\\'):
            cstart = string.find(line, "%", cstart+1)
        if cstart > 0:
            return line[:(cstart-1)]
        return line


    def BlankLine(self):
        if self.inDocument:
            self.writer.ParagraphBreak()

    def WriteWord(self, word):
        if len(word) == 0:
            return 0
        if word[0] == '\\':
            matchLen = self.command2Regex.match(word)
            if matchLen > 0:
                self.WriteCommand2(self.command2Regex.group(1),
                                   self.command2Regex.group(2),
                                   self.command2Regex.group(3))
                if len(word) > matchLen:
                    self.WriteWord(word[matchLen:])
                return 1                
            matchLen = self.commandRegex.match(word)
            if matchLen > 0:
                self.WriteCommand(self.commandRegex.group(1),
                                  self.commandRegex.group(2))
                if len(word) > matchLen:
                    self.WriteWord(word[matchLen:])
                return 1

        if (len(word) == 4) and (word[:2] == "{\\"):
            self.WriteFormatStart(word)
            return 0
        p = string.find(word, "\\")
        if p > 0:
            self.WriteWord(word[:p])
            self.WriteWord(word[p:])
            return 1
        p = string.find(word, "}")
        if p >= 0:
            if p > 0:
                self.WriteWord(word[:p])
            self.WriteFormatEnd()
            if len(word) > (p+1):
                self.WriteWord(word[(p+1):])
            return 1
        if word == "---":
            self.writer.EmDash()
            return 1
        if self.HandleSpecialChar(word, "``", self.writer.LeftDoubleQuote):
            return 1
        if self.HandleSpecialChar(word, "''", self.writer.RightDoubleQuote):
            return 1
        if self.HandleSpecialChar(word, "$mu$",
                                  lambda s=self: s.writer.ParagraphText("\xb5")):
            return 1
        self.writer.ParagraphText(word)
        return 1


    def HandleSpecialChar(self, word, chars, funct):
        p = string.find(word, chars)
        if p >= 0:
            le = len(chars)
            if p > 0:
                self.WriteWord(word[:p])
            funct()
            if len(word) > (p+le):
                self.WriteWord(word[(p+le):])
            return 1
        return 0


    def WriteFormatStart(self, word):
        if word[2:] == "bf":
            self.inBold = 1
            self.writer.BeginBoldText()
            return
        elif word[2:] == "it":
            self.inItalics = 1
            self.writer.BeginItalicsText()
            return
        elif word[2:] == "em":
            self.inEmphasis = 1
            self.writer.BeginEmphasizedText()
            return
        self.writer.ParagraphText(word)


    def WriteFormatEnd(self):
        if self.inEmphasis:
            self.inEmphasis = 0
            self.writer.EndEmphasizedText()
            return
        elif self.inItalics:
            self.inItalics = 0
            self.writer.EndItalicsText()
            return
        elif self.inBold:            
            self.inBold = 0
            self.writer.EndBoldText()
            return
        self.writer.ParagraphText("}")


    def WriteCommand(self, command, argument):
        if command == "mFileName":
            self.writer.BeginTeletype()
            self.writer.ParagraphText(argument)
            self.writer.EndTeletype()
            return
        elif command == "mResWord":
            self.writer.BeginTeletype()
            self.writer.ParagraphText(argument)
            self.writer.EndTeletype()
            return
        elif command == "mVariable":
            self.writer.BeginBoldText()
            self.writer.ParagraphText(argument)
            self.writer.EndBoldText()
            return
        elif command == "mDocRef":
            self.writer.LinkText(argument, argument)
            return
        elif command == "mApiRef":
            self.writer.LinkText(argument, argument)
        else:
            self.writer.ParagraphText("\\" + command + "{" + argument + "}")


    def WriteCommand2(self, command, argument1, argument2):
        if command == "mDocRefLabel":
            self.writer.LinkText(argument2, argument1)
            return
        elif command == "mApiRefLabel":
            self.writer.LinkText(argument2, argument1)
            return
        else:
            self.writer.ParagraphText("\\" + command + "{" + argument1 + "}"
                                      "{" + argument2 + "}")



            
    def CleanWord(self, word):
        return string.joinfields(string.splitfields(string.joinfields(
            string.splitfields(string.joinfields(
                string.splitfields(word, "\\#{}"), '#'), "\\%{}"), "%"),
                "\\_{}"), "_")
                

    def WriteLine(self, line):
        if not self.inSection:
            return
        
        if self.inVerbatim:
            self.writer.VerbatimLine(line)
        else:
            words = string.split(line)
            for w in range(len(words)):
                #self.WriteWord(string.strip(words[w]))
                if self.WriteWord(self.CleanWord(string.strip(words[w]))):
                    self.writer.ParagraphText(" ")

    def ConvertLine(self, line):
        if self.inDocument:
            if line[0] == '\\':
                if self.itemRegex.match(line) > 0:
                    self.writer.DescriptionListItem(self.CleanWord(
                        self.itemRegex.group(1)))
                    self.WriteLine(line[self.itemRegex.match(line):])
                    return
                if self.beginRegex.match(line) > 0:
                    name = self.beginRegex.group(1)
                    if name == "verbatim":
                        self.inVerbatim = 1
                        self.writer.BeginVerbatim()
                        return
                    elif name == "description":
                        self.writer.BeginDescriptionList()
                        return
                    else:
                        self.WriteLine(line)
                        return
                elif self.endRegex.match(line) > 0:
                    name = self.endRegex.group(1)
                    if name == "verbatim":
                        self.inVerbatim = 0
                        self.writer.EndVerbatim()
                        return
                    elif name == "description":
                        self.writer.EndDescriptionList()
                        return
                    else:
                        self.WriteLine(line)
                        return
                elif self.chapterRegex.match(line) > 0:
                    self.writer.NewSection(2, self.chapterRegex.group(1),
                                           "section" + str(self.sectionNumber),
                                           self.GetKeywords())
                    self.sectionNumber = self.sectionNumber + 1
                    self.inSection = 1
                    return
                elif self.sectionRegex.match(line) > 0:
                    self.writer.NewSection(3, self.sectionRegex.group(1),
                                           "section" + str(self.sectionNumber),
                                           self.GetKeywords())
                    self.sectionNumber = self.sectionNumber + 1
                    self.inSection = 1
                    return
                elif self.subsectionRegex.match(line) > 0:
                    self.writer.NewSection(4, self.subsectionRegex.group(1),
                                           "section" + str(self.sectionNumber),
                                           self.GetKeywords())
                    self.sectionNumber = self.sectionNumber + 1
                    self.inSection = 1
                    return
                elif self.mDocEntryRegex.match(line) > 0:
                    keywords = self.GetKeywords()
                    name = self.mDocEntryRegex.group(1)
                    if keywords != "":
                        keywords = name + ";" + keywords
                    else:
                        keywords = name
                    self.writer.NewSection(4, name, name, keywords)
                    self.sectionNumber = self.sectionNumber + 1
                    self.inSection = 1
                    return
                elif self.paragraphTitleRegex.match(line) > 0:
                    self.writer.ParagraphTitle(self.paragraphTitleRegex.
                                               group(1))
                    return
                else:
                    self.WriteLine(line)
                    return
            else:
                self.WriteLine(line)
                return
        else:
            if self.titleRegex.match(line) > 0:
                title = self.titleRegex.group(1)
                a = string.rfind(title, '\\')
                if a > 0:
                    title = string.strip(title[a+1:])
                self.writer.NewSection(1, title)
                return
            if line == "\\begin{document}":
                self.inDocument = 1


    def GetKeywords(self):
        p = string.find(self.sourceLine, "%K:")
        if p < 0:
            return ""
        keywords = ""
        line = self.sourceLine[p+3:]
        while line[-1] == '\\':
            keywords = keywords + line[:-1]
            line = string.strip(self.inf.readline())[3:]
            while (len(line) > 0) and (line[-1] == '\012'):
                line = line[:-1]
        keywords = keywords + line
        return keywords


# Da main program:

if len(sys.argv) < 4:
    print "Usage: docconv.py type targetname sources"
    print " type: (t)ext or (w)inhelp"
    sys.exit(1)

if sys.argv[1] == "t":
    writer = PlainTextWriter(sys.argv[2])
    writer.NewFile(sys.argv[2])
    converter = MdocConverter(writer)
    for name in sys.argv[3:]:
        print "Converting " + name
        converter.ConvertFile(name)
    del converter
    del writer

elif sys.argv[1] == "w":
    writer = WinhelpWriter(sys.argv[2])
    writer.NewFile(sys.argv[2])
    converter = MdocConverter(writer)
    for name in sys.argv[3:]:
        print "Converting " + name
        converter.ConvertFile(name)
    del converter
    del writer


# $Log: docconv.py,v $
# Revision 1.6  1997/08/15 13:18:54  pekangas
# Added conversion of $mu$s
#
# Revision 1.5  1997/07/31 10:56:59  pekangas
# Renamed from MIDAS Sound System to MIDAS Digital Audio System
#
# Revision 1.4  1997/07/29 20:31:06  pekangas
# Added sys.stdout.flush()es to keep output visible in Linux
#
# Revision 1.3  1997/07/29 19:38:55  pekangas
# Added plain text output
#
# Revision 1.2  1997/05/20 20:34:20  pekangas
# Added conversion for left and right double quotes and em-dashes
#
# Revision 1.1  1997/05/08 12:11:31  pekangas
# Initial revision
#

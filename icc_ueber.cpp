/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
 *
 * Autor: Kai-Uwe Behrmann <ku.b@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * -----------------------------------------------------------------------------
 *
 * Inhalt fuer das informative "Über" Fenster.
 * 
 */


#include "icc_utils.h"
#include "icc_examin_version.h"
//#include "icc_ueber.h"
#include <sstream>


#define DEBUG_ICCUEBER

#define  SF "#cccccc"  // Standard hintergrundFarbe

# define LAYOUTFARBE if (layout[l++] == true) \
                        html << HF; \
                      else \
                        html << SF; //Farbe nach Layoutoption auswaehlen

std::string
getHtmlKopf(const char* titel)
{
  std::stringstream html;
  html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
       << "<meta http-equiv=\"Content-Type\" content=\"text/html\"; charset=ISO-8859-1>"
       << "</html><head>\n"
       << "<title>" << titel << "</title>\n"
       << "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">\n"
       << "<meta name=\"description\" content=\"icc_examin ICC Profil Betrachter\">\n"
       << "<meta name=\"author\" content=\"automatic generated by icc_examin-" << ICC_EXAMIN_V << "\">\n"
       << "</head><body bgcolor=\"" << SF << "\" text=\"#000000\" link=\"blue\">\n\n";
  return html.str();
}


std::string
getUeberHtml()
{
  DBG_PROG_START
  //char HF[] = "#aaaaaa";  // Hervorgehoben
  std::stringstream html;

  html << getHtmlKopf( _("About ICC Examin") );

  // Allgemeine Informationen
  html <<
"<table cellspacing=3 cellpadding=4 width=\"100%\"> \
  <tr> \
    <td bgcolor=\"" << SF << "\"> \
    <center> \
    <h2>"
       << "  " << _("ICC Examin Version") << " " << ICC_EXAMIN_V  << " \
    </h2> \
    </center><br><br><br>. \
    </td> \
  </tr> \
  <tr> \
    <td bgcolor=\"" << "#9f9f9f" << "\"> \
    <b>Copyright &copy; 2004-2006  Kai-Uwe Behrmann \
    </b><p> \
      <table cellpadding=0 cellspacing=0 width=\"100%\"> \
        <tr> \
          <td width=\"65%\"> \
          </td> \
          <td valign=top> \
          </td> \
        </tr> \
      </table> \
<br>";
  html << _("Date") << ": " << ICC_EXAMIN_D; html << " \
<br> \
<p>";
  html << _(
"internet: www.behrmann.name <br>");
  html << " \
</p>\n \
<p>";
  html << _(
"ICC Examin is a viewer for colour profiles according to ICC "
"standard  - www.color.org . At the moment version 2 profiles are supported.");
  html << " \
</p>\n \
<p>";
  html << _(
"The appliance of colour profiles shall help to achive an easy colour data "
"exchange, regardless which application or what kind of operating system is "
"used. As well colour adaption between different devices is possible, "
"provided the colour characteristics of both devices are known. "
"The qulity of a colour transform from one to an other device depends mainly "
"on the quality of the colour measurement and the used profiling algorithm "
"during profile creation.");
  html << " \
</p>\n \
<p>";
  html << _(
"Colour profiles are often available from manufacturers of optical devices "
"(digital camera, scanner, printer). With ICC Examin you can get an impression "
"of the internal data , stored in a ICC profile. For profiles containing "
"measurement data, ICC Examin can generate a quality report. The deviation, "
"calculated from the measurement data is shown as well in the three "
"dimensional gamut viewer.<br>");
  html << " \
</p>\n\
<p>";
  html << _(
"The internal colour transformations are currently realised with functions "
"of the littleCMS program library.");
  html << " \
</p>\n";

  html << "\n\n<br>\n</body></html>\n";

  //DBG_NUM_S(html.str() )
  DBG_PROG_ENDE
  return html.str();
}

std::string
getHilfeHtml()
{
  DBG_PROG_START
  std::stringstream html;
  html << getHtmlKopf( _("Help") );

  html << " \
<p>ICC Examin is a FLTK based colour profile viewer. \
</p><p>The educational purpose is to show the otherwise hidden internals of ICC profiles and give users a rough estimation, what the profiles can be used for. \
</p> \
<table border=\"0\" id=\"toc\"><tr id=\"toctitle\"><td align=\"center\"> \
<b>Table of contents</b></td></tr><tr id=\"tocinside\"><td> \
<div class=\"tocline\"><a href=\"#Features\">1 Features</a><br /></div> \
<div class=\"tocline\"><a href=\"#Usage\">2 Usage</a><br /></div> \
 \
<div class=\"tocindent\"> \
<p><a href=\"#File_opening\">2.1 File opening</a><br /> \
<a href=\"#Main_window\">2.2 Main window</a><br /> \
<a href=\"#Complex_tags_navigation\">2.3 Complex tags navigation</a><br /> \
<a href=\"#3D_navigation\">2.4 3D navigation</a><br /> \
<a href=\"#Windows_and_Views\">2.5 Windows and Views</a><br /> \
</p> \
<div class=\"tocindent\"> \
<p><a href=\"#Gamut\">2.5.1 Gamut</a><br /> \
<a href=\"#Report_View\">2.5.2 Report View</a><br /> \
 \
<a href=\"#CGATS_view\">2.5.3 CGATS view</a><br /> \
<a href=\"#Grafic_card_gamma_tables\">2.5.4 Grafic card gamma tables</a><br /> \
<a href=\"#Help\">2.5.5 Help</a><br /> \
</p> \
</div> \
</div> \
<div class=\"tocline\"><a href=\"#Profile_elements\">3 Profile elements</a><br /></div> \
<div class=\"tocindent\"> \
<p><a href=\"#Header\">3.1 Header</a><br /> \
<a href=\"#Tags\">3.2 Tags</a><br /> \
</p> \
 \
<div class=\"tocindent\"> \
<p><a href=\"#text_.2F_desc_.2F_targ\">3.2.1 text / desc / targ</a><br /> \
<a href=\"#curv\">3.2.2 curv</a><br /> \
<a href=\"#XYZ\">3.2.3 XYZ</a><br /> \
<a href=\"#mft1_.2F_mft2\">3.2.4 mft1 / mft2</a><br /> \
<a href=\"#sf32\">3.2.5 sf32</a><br /> \
<a href=\"#ncl2\">3.2.6 ncl2</a><br /> \
</p> \
</div> \
</div> \
<div class=\"tocline\"><a href=\"#Other_file_formats\">4 Other file formats</a><br /></div> \
 \
<div class=\"tocindent\"> \
<p><a href=\"#CGATS\">4.1 CGATS</a><br /> \
<a href=\"#VRML\">4.2 VRML</a><br /> \
</p> \
</div> \
</td></tr></table> \
<a name=\"Features\"></a><h2> Features </h2> \
<ul><li> see the <a href=\"http://www.behrmann.name/index.php?option=com_content&amp;task=view&amp;id=32&amp;Itemid=1\" class='external' title=\"http://www.behrmann.name/index.php?option=com content&amp;task=view&amp;id=32&amp;Itemid=1\">ICC Examin</a><span class='urlexpansion'>&nbsp;(<i>http://www.behrmann.name</i>)</span> page for an overview \
 \
</li></ul> \
<a name=\"Usage\"></a><h2> Usage </h2> \
<p>ICC Examin can be launched from the Application folder or Dock on osX and from the Graphics menu on Linux. \
</p> \
<a name=\"File_opening\"></a><h3> File opening </h3> \
<p>From the application windows menubar you can select File-&gt;Open and to show file dialog. Select one or more files for gamut comparision as you like. On top right side of the FLTK file selector resides the bookmark alias Favourites button. If you frequently visit directories you will find it usefull to append these directories to your bookmark / favourites list. To do this, go to an directory with the file selector and choose from the bookmark button the add entry. The added directory appears in the bookmark list and is available for later use. \
 \
</p><p>You have selected one file and pressed ok. ICC Examin starts to parse the file, showing the yellow progressbar. Depending on the complexity of the file it may take some time to finish. \
</p> \
<a name=\"Main_window\"></a><h3> Main window </h3> \
<p>In normal mode ICC Examin shows first the tag list with the tag viewer below. Over the tag list resides the applications menubar. \
</p><p>You can select an tag from the taglist and watch its content in the tag window below. \
</p><p>On the bottom you see the status area of the window. \
</p> \
<a name=\"Complex_tags_navigation\"></a><h3> Complex tags navigation </h3> \
 \
<p>Some tags contain complex data. For such tags may appear a choicebutton below the tag list. There you can select detailed views of a complex tags endities. \
</p> \
<a name=\"3D_navigation\"></a><h3> 3D navigation </h3> \
<p>To navigate arround in a 3D view use the: \
</p> \
<ul><li> left mouse button to rotate \
</li><li> middle mouse button or Shift+left mouse button to move/clip \
</li><li> right mouse button/ Strg+left mouse button to obtain additional menus. Among the additional menus are in: \
<ul><li> top menu: the channel selection if appropriate \
 \
<ul><li> Slice plane submenu, with axis slicing in fly through style and rotation \
</li><li> Illustration submenu, with the background colour selection and text/marker switching off \
</li></ul> \
</li></ul> \
</li></ul> \
<a name=\"Windows_and_Views\"></a><h3> Windows and Views  </h3> \
<a name=\"Gamut\"></a><h4> Gamut </h4> \
 \
<p>First to call is the 3D gamut viewer. You can select it from the menu bar. It shows the gamut of the device described by the profile. If such a description is not appropriate, the external used Argyll iccgamut utility may not deliver a vrml gamut file and thus the 3D gamut view may remain empty.  \
</p><p>The colour space shown is allways in CIE*Lab coordinates.  \
</p><p>From the context menu (ctrl+left mouse button or right mouse button) you can select the average human (CIE standard observer) visible spectral colours as a line. \
</p><p>If meashurements are shown, theyr radius can be selected from the context menu. This is handy to easily detect dE's of above 2 or 4. The connection line goes from the measurement (white) to the deviation by the profile (red). \
</p><p>ncl2 colour spots can be enlarged or made smaller by pressing + or -. \
</p> \
<a name=\"Report_View\"></a><h4> Report View </h4> \
<p>If the profile contains measurements a report is generated. You reach this by selecting from the top menu View-&gt;Report. These measurement data are compared to what is calculated with the profile by littleCMS - the internal used CMM. \
</p><p>Such a comparision from device colours + measurements colours on the one side and device + profile/CMM colours on the other side are just one possible quality checking method. \
</p><p>More comparision styles would be nice in giving better meaning for varying criteria. The differences are summarised in the reports top lines. Each like contains the the colour values and euclidian dE + dE2000. Two rectangles on the right side may help to see the colour differences visually. The first monitor profile is used for this. \
</p><p>The report can be exported from the File menu in html format. \
 \
</p> \
<a name=\"CGATS_view\"></a><h4> CGATS view </h4> \
<p>The measurement data is taken from the profile, if available, and transformed to fit in the CGATS standard. A window can be called from the View menu to show the corrected measurement output. \
</p> \
<a name=\"Grafic_card_gamma_tables\"></a><h4> Grafic card gamma tables </h4> \
<p>The gamma tables in the grafic card are shown in an external window, callable from the View menu. It shows on osX/XFree86/Xorg the RGB curves. The monitor profile can be loaded from this window in the main profile viewer window for further examination.  \
</p><p>Under Linux Oyranos support must be compiled in to do so. \
 \
</p> \
<a name=\"Help\"></a><h4> Help </h4> \
<p>The Help menus About button shows a short overview, license and other programers acknowledgement, who helped with providing theyre code for free. \
</p> \
<a name=\"Profile_elements\"></a><h2> Profile elements </h2> \
<a name=\"Header\"></a><h3> Header </h3> \
 \
<p>Now you see the tag list has obtained some entries, showing the profiles tags. The first entry is allways the header. If the header is selected the tag window below shows many useful information about the profile. Among these information are: \
</p> \
<ul><li> the internal stored size of the profile \
</li><li> the prefered ColourMatchingModule (CMM) \
</li><li> ICC standard version \
</li><li> kind of usage \
</li><li> device colour space \
</li><li> the internal used reference colour space \
</li><li> date of creation \
</li><li> file type sinature (allways acsp) \
 \
</li><li> and some more or less meaningfull informations \
</li></ul> \
<a name=\"Tags\"></a><h3> Tags </h3> \
<p>Below the header, which behaves like a fixed tag, are the variable tags listed. They appear in the same order as in the file. Starting withe the tag number, then the tag signature and the type of content in the tag. The size of the tag follows and a small description gives you an idea af what the tag may be intended for. \
</p> \
<a name=\"text_.2F_desc_.2F_targ\"></a><h4> text / desc / targ </h4> \
 \
<p>Tags of type text are the most simple ones. they include informations about the profile like License informations, Description for displaying on behalf of the whole profile, Measurement data , in CGATS text formet and more. \
</p> \
<a name=\"curv\"></a><h4> curv </h4> \
<p>An other type are curves. They are shown as such. Note the coordinates are all normalised to 0.0 -&gt; 1.0. \
</p> \
<a name=\"XYZ\"></a><h4> XYZ </h4> \
 \
<p>XYZ tags show things like Mediawhitepoint or Primaries of monitors in the CIE*xy diagram.  \
</p><p>Note: primaries and some curves are grouped to better understand theyr meaning in the profile. \
</p> \
<a name=\"mft1_.2F_mft2\"></a><h4> mft1 / mft2 </h4> \
<p>MFT1/2 tags are complex and contain a set of peaces to do a colour transformation. The choice button below the tag view names them. You can select the matrix, in-and output curves and the CLUT. Currently only CLUTs with less than 3 dimensions in input direction are supported. This kind of tag is mostly independend of other tags. Only few tags like wtpt and chad may influence them.  \
</p><p>The CLUT view has various options to visualise the table. For instance you can make table appear coloured or select a channel of choice. The numbers in the status window are normalised to 0.0 -&gt; 1.0. \
</p><p>They contain the result of hard work in researching the best translation of scattered measurements to equally spaced grid tables. Only this step makes colour transformation reasonably fast.  \
</p><p>The tag is described in in verson 2 of the ICC specification. In ICC specification version 4 a new even more complex tag is introduced and is calleed mAB. \
</p> \
<a name=\"sf32\"></a><h4> sf32 </h4> \
 \
<p>sf32 and similiar tags may contain only numbers. Theyre interpretation depends on the name of the tag. For instance the tag named chad is used to specify a colour transformation matrix, which was originally used to normalise the measured colours to D50 standard conditions. \
</p> \
<a name=\"ncl2\"></a><h4> ncl2 </h4> \
<p>The ncl2 tag is used to store named colours in a list. This can be used to describe spot colours or describe a painters palette. Loading a profile containing such a tag into ICC Examin switches the 3D gamut view mode on. You see then the colours in 3D (CIE*Lab) and not as a list of numbers as in the tag viewer. \
</p><p><br /> \
More tags can follow here... \
</p> \
<a name=\"Other_file_formats\"></a><h2> Other file formats </h2> \
 \
<a name=\"CGATS\"></a><h3> CGATS </h3> \
<p>Measurement data in CGATS format can be opened in ICC Examin. In the file selector is a dedicated filter available. The data are shown as text and in the 3D view. \
</p> \
<a name=\"VRML\"></a><h3> VRML </h3> \
<p>A subset of vrml alias wrl files is parsed. It allowes to open colour gamuts produced with argyll and saved with ICC Examin.  \
</p><p>It would be nice to allow as well lines to get parsed, which are used for visualisations by iccview for instance. \
</p>";
  html << "\n\n<br>\n</body></html>\n";

  DBG_PROG_ENDE
  return html.str();
}

std::string
getLizenzHtml()
{
  DBG_PROG_START
  std::stringstream html;
  html << getHtmlKopf( _("License") );

  html << " \
<p>\n";
  html << _(
"    ICC Examin is licensed under the GPL version 2."
"    Especially all icc_* files are licensed under the GPL.<br>"     
"    The other source files may contain devergine licenses, which are detectable from "
"file headers. ");
  html << " \
<br<br> \
</p>\n";

  html << " \
<HR> \
 \
 \
<H2><A NAME=\"SEC1\" HREF=\"http://www.gnu.org/licenses/gpl.html\">GNU GENERAL PUBLIC LICENSE</A></H2> \
<P> \
Version 2, June 1991 \
 \
</P> \
 \
Copyright (C) 1989, 1991 Free Software Foundation, Inc.   \
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA \
 \
Everyone is permitted to copy and distribute verbatim copies \
of this license document, but changing it is not allowed. \
 \
 \
 \
<H2><A NAME=\"SEC2\" HREF=\"http://www.gnu.org/licenses/gpl.html#TOC2\">Preamble</A></H2> \
 \
<P> \
  The licenses for most software are designed to take away your \
freedom to share and change it.  By contrast, the GNU General Public \
License is intended to guarantee your freedom to share and change free \
software--to make sure the software is free for all its users.  This \
General Public License applies to most of the Free Software \
Foundation's software and to any other program whose authors commit to \
using it.  (Some other Free Software Foundation software is covered by \
the GNU Library General Public License instead.)  You can apply it to \
your programs, too. \
 \
</P> \
<P> \
  When we speak of free software, we are referring to freedom, not \
price.  Our General Public Licenses are designed to make sure that you \
have the freedom to distribute copies of free software (and charge for \
this service if you wish), that you receive source code or can get it \
if you want it, that you can change the software or use pieces of it \
in new free programs; and that you know you can do these things. \
 \
</P> \
<P> \
  To protect your rights, we need to make restrictions that forbid \
anyone to deny you these rights or to ask you to surrender the rights. \
These restrictions translate to certain responsibilities for you if you \
distribute copies of the software, or if you modify it. \
 \
</P> \
<P> \
  For example, if you distribute copies of such a program, whether \
gratis or for a fee, you must give the recipients all the rights that \
you have.  You must make sure that they, too, receive or can get the \
source code.  And you must show them these terms so they know their \
rights. \
 \
</P> \
<P> \
  We protect your rights with two steps: (1) copyright the software, and \
(2) offer you this license which gives you legal permission to copy, \
distribute and/or modify the software. \
 \
</P> \
<P> \
  Also, for each author's protection and ours, we want to make certain \
that everyone understands that there is no warranty for this free \
software.  If the software is modified by someone else and passed on, we \
want its recipients to know that what they have is not the original, so \
that any problems introduced by others will not reflect on the original \
authors' reputations. \
 \
</P> \
<P> \
  Finally, any free program is threatened constantly by software \
patents.  We wish to avoid the danger that redistributors of a free \
program will individually obtain patent licenses, in effect making the \
program proprietary.  To prevent this, we have made it clear that any \
patent must be licensed for everyone's free use or not licensed at all. \
 \
</P> \
<P> \
  The precise terms and conditions for copying, distribution and \
modification follow. \
 \
</P> \
 \
 \
<H2><A NAME=\"SEC3\" HREF=\"gpl.html#TOC3\">TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION</A></H2> \
 \
 \
<P> \
 \
<STRONG>0.</STRONG> \
 This License applies to any program or other work which contains \
a notice placed by the copyright holder saying it may be distributed \
under the terms of this General Public License.  The \"Program\", below, \
refers to any such program or work, and a \"work based on the Program\" \
means either the Program or any derivative work under copyright law: \
that is to say, a work containing the Program or a portion of it, \
either verbatim or with modifications and/or translated into another \
language.  (Hereinafter, translation is included without limitation in \
the term \"modification\".)  Each licensee is addressed as \"you\". \
<P> \
 \
Activities other than copying, distribution and modification are not \
covered by this License; they are outside its scope.  The act of \
running the Program is not restricted, and the output from the Program \
is covered only if its contents constitute a work based on the \
Program (independent of having been made by running the Program). \
Whether that is true depends on what the Program does. \
 \
<P> \
 \
<STRONG>1.</STRONG> \
 You may copy and distribute verbatim copies of the Program's \
source code as you receive it, in any medium, provided that you \
conspicuously and appropriately publish on each copy an appropriate \
copyright notice and disclaimer of warranty; keep intact all the \
notices that refer to this License and to the absence of any warranty; \
and give any other recipients of the Program a copy of this License \
along with the Program. \
<P> \
 \
You may charge a fee for the physical act of transferring a copy, and \
you may at your option offer warranty protection in exchange for a fee. \
<P> \
 \
<STRONG>2.</STRONG> \
 You may modify your copy or copies of the Program or any portion \
of it, thus forming a work based on the Program, and copy and \
distribute such modifications or work under the terms of Section 1 \
above, provided that you also meet all of these conditions: \
<P> \
 \
<UL> \
 \
<LI><STRONG>a)</STRONG> \
     You must cause the modified files to carry prominent notices \
     stating that you changed the files and the date of any change. \
 \
<P> \
<LI><STRONG>b)</STRONG> \
     You must cause any work that you distribute or publish, that in \
     whole or in part contains or is derived from the Program or any \
     part thereof, to be licensed as a whole at no charge to all third \
     parties under the terms of this License. \
 \
<P> \
<LI><STRONG>c)</STRONG> \
     If the modified program normally reads commands interactively \
     when run, you must cause it, when started running for such \
     interactive use in the most ordinary way, to print or display an \
     announcement including an appropriate copyright notice and a \
     notice that there is no warranty (or else, saying that you provide \
     a warranty) and that users may redistribute the program under \
     these conditions, and telling the user how to view a copy of this \
     License.  (Exception: if the Program itself is interactive but \
     does not normally print such an announcement, your work based on \
     the Program is not required to print an announcement.) \
</UL> \
 \
These requirements apply to the modified work as a whole.  If \
identifiable sections of that work are not derived from the Program, \
and can be reasonably considered independent and separate works in \
themselves, then this License, and its terms, do not apply to those \
sections when you distribute them as separate works.  But when you \
distribute the same sections as part of a whole which is a work based \
on the Program, the distribution of the whole must be on the terms of \
this License, whose permissions for other licensees extend to the \
entire whole, and thus to each and every part regardless of who wrote it. \
<P> \
 \
Thus, it is not the intent of this section to claim rights or contest \
your rights to work written entirely by you; rather, the intent is to \
exercise the right to control the distribution of derivative or \
collective works based on the Program. \
<P> \
 \
In addition, mere aggregation of another work not based on the Program \
with the Program (or with a work based on the Program) on a volume of \
a storage or distribution medium does not bring the other work under \
the scope of this License. \
 \
<P> \
 \
<STRONG>3.</STRONG> \
 You may copy and distribute the Program (or a work based on it, \
under Section 2) in object code or executable form under the terms of \
Sections 1 and 2 above provided that you also do one of the following: \
 \
 \
<!-- we use this doubled UL to get the sub-sections indented, --> \
<!-- while making the bullets as unobvious as possible. --> \
<UL> \
 \
<LI><STRONG>a)</STRONG> \
     Accompany it with the complete corresponding machine-readable \
     source code, which must be distributed under the terms of Sections \
     1 and 2 above on a medium customarily used for software interchange; or, \
 \
<P> \
<LI><STRONG>b)</STRONG> \
     Accompany it with a written offer, valid for at least three \
     years, to give any third party, for a charge no more than your \
     cost of physically performing source distribution, a complete \
     machine-readable copy of the corresponding source code, to be \
     distributed under the terms of Sections 1 and 2 above on a medium \
     customarily used for software interchange; or, \
 \
<P> \
<LI><STRONG>c)</STRONG> \
     Accompany it with the information you received as to the offer \
     to distribute corresponding source code.  (This alternative is \
     allowed only for noncommercial distribution and only if you \
     received the program in object code or executable form with such \
     an offer, in accord with Subsection b above.) \
</UL> \
 \
The source code for a work means the preferred form of the work for \
making modifications to it.  For an executable work, complete source \
code means all the source code for all modules it contains, plus any \
associated interface definition files, plus the scripts used to \
control compilation and installation of the executable.  However, as a \
special exception, the source code distributed need not include \
anything that is normally distributed (in either source or binary \
form) with the major components (compiler, kernel, and so on) of the \
operating system on which the executable runs, unless that component \
itself accompanies the executable. \
<P> \
 \
If distribution of executable or object code is made by offering \
access to copy from a designated place, then offering equivalent \
access to copy the source code from the same place counts as \
distribution of the source code, even though third parties are not \
compelled to copy the source along with the object code. \
<P> \
 \
<STRONG>4.</STRONG> \
 You may not copy, modify, sublicense, or distribute the Program \
except as expressly provided under this License.  Any attempt \
otherwise to copy, modify, sublicense or distribute the Program is \
void, and will automatically terminate your rights under this License. \
However, parties who have received copies, or rights, from you under \
this License will not have their licenses terminated so long as such \
parties remain in full compliance. \
 \
<P> \
 \
<STRONG>5.</STRONG> \
 You are not required to accept this License, since you have not \
signed it.  However, nothing else grants you permission to modify or \
distribute the Program or its derivative works.  These actions are \
prohibited by law if you do not accept this License.  Therefore, by \
modifying or distributing the Program (or any work based on the \
Program), you indicate your acceptance of this License to do so, and \
all its terms and conditions for copying, distributing or modifying \
the Program or works based on it. \
 \
<P> \
 \
<STRONG>6.</STRONG> \
 Each time you redistribute the Program (or any work based on the \
Program), the recipient automatically receives a license from the \
original licensor to copy, distribute or modify the Program subject to \
these terms and conditions.  You may not impose any further \
restrictions on the recipients' exercise of the rights granted herein. \
You are not responsible for enforcing compliance by third parties to \
this License. \
 \
<P> \
 \
<STRONG>7.</STRONG> \
 If, as a consequence of a court judgment or allegation of patent \
infringement or for any other reason (not limited to patent issues), \
conditions are imposed on you (whether by court order, agreement or \
otherwise) that contradict the conditions of this License, they do not \
excuse you from the conditions of this License.  If you cannot \
distribute so as to satisfy simultaneously your obligations under this \
License and any other pertinent obligations, then as a consequence you \
may not distribute the Program at all.  For example, if a patent \
license would not permit royalty-free redistribution of the Program by \
all those who receive copies directly or indirectly through you, then \
the only way you could satisfy both it and this License would be to \
refrain entirely from distribution of the Program. \
<P> \
 \
If any portion of this section is held invalid or unenforceable under \
any particular circumstance, the balance of the section is intended to \
apply and the section as a whole is intended to apply in other \
circumstances. \
<P> \
 \
It is not the purpose of this section to induce you to infringe any \
patents or other property right claims or to contest validity of any \
such claims; this section has the sole purpose of protecting the \
integrity of the free software distribution system, which is \
implemented by public license practices.  Many people have made \
generous contributions to the wide range of software distributed \
through that system in reliance on consistent application of that \
system; it is up to the author/donor to decide if he or she is willing \
to distribute software through any other system and a licensee cannot \
impose that choice. \
<P> \
 \
This section is intended to make thoroughly clear what is believed to \
be a consequence of the rest of this License. \
 \
<P> \
 \
<STRONG>8.</STRONG> \
 If the distribution and/or use of the Program is restricted in \
certain countries either by patents or by copyrighted interfaces, the \
original copyright holder who places the Program under this License \
may add an explicit geographical distribution limitation excluding \
those countries, so that distribution is permitted only in or among \
countries not thus excluded.  In such case, this License incorporates \
the limitation as if written in the body of this License. \
 \
<P> \
 \
<STRONG>9.</STRONG> \
 The Free Software Foundation may publish revised and/or new versions \
of the General Public License from time to time.  Such new versions will \
be similar in spirit to the present version, but may differ in detail to \
address new problems or concerns. \
<P> \
 \
Each version is given a distinguishing version number.  If the Program \
specifies a version number of this License which applies to it and \"any \
later version\", you have the option of following the terms and conditions \
either of that version or of any later version published by the Free \
Software Foundation.  If the Program does not specify a version number of \
this License, you may choose any version ever published by the Free Software \
Foundation. \
 \
<P> \
 \
 \
<STRONG>10.</STRONG> \
 If you wish to incorporate parts of the Program into other free \
programs whose distribution conditions are different, write to the author \
to ask for permission.  For software which is copyrighted by the Free \
Software Foundation, write to the Free Software Foundation; we sometimes \
make exceptions for this.  Our decision will be guided by the two goals \
of preserving the free status of all derivatives of our free software and \
of promoting the sharing and reuse of software generally. \
 \
 \
 \
<P><STRONG>NO WARRANTY</STRONG></P> \
 \
<P> \
 \
<STRONG>11.</STRONG> \
 BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY \
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN \
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES \
PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED \
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF \
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS \
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE \
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, \
REPAIR OR CORRECTION. \
 \
<P> \
 \
<STRONG>12.</STRONG> \
 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING \
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR \
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, \
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING \
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED \
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY \
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER \
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE \
POSSIBILITY OF SUCH DAMAGES. \
 \
<P> \
 \
 \
<H2>END OF TERMS AND CONDITIONS</H2> \
 \
 \
 \
";

  html << "\n\n<br>\n</body></html>\n";

  DBG_PROG_ENDE
  return html.str();
}

std::string
getDankHtml()
{
  DBG_PROG_START
  std::stringstream html;
  html << getHtmlKopf( _("Acknowledgement") );

  html << "\
<p>";
  html << _(
"My cordially thank is especially dedicated to Marti Maria and the many "
"writers of the Lcms-user email list.");
  html << " \
</p>\n \
<br> \
<br> \
    </font> \
    </font> \
    </td> \
  </tr> \
</table>\
<p>\n";
  html << _(
"ICC Examin uses following libraries:<br>\n");
  html << " \
<p>";
  html << _(
"  FLTK: Bill Spitzack and others www.fltk.org<br>\n");
#ifdef HAVE_FLU
  html << " \
</p>\n \
<p>";
  html << _(
"  FLU: Jason Bryan www.osc.edu/~jbryan/FLU/<br>\n");
#endif
  html << " \
</p>\n \
<p>";
  html << _(
"  littleCMS: Marti Maria www.littlecms.com<br>\n");
  html << " \
 <br> \
 <br> \
</p>\n \
<br>\n";
#ifdef HAVE_FTGL
  html << " \
<p>";
  html << _(
"  FTGL: Henry Maddocks homepages.paradise.net.nz/henryj/code/#FTGL<br>\n");
  html << " \
</p>";
#endif
  html << " \
<p>";
  html << _(
"integrated sources, lock at the according files for licence conditions:<br>\n");
  html << " \
<p>";
  html << _(
"  agv 3D Navigation: Philip Winston<br>\n");
  html << " \
</p>\n \
<p>";
  html << _(
"For the visualisation of gamut (external):<br>\n");
  html << " \
<p>";
  html << _( 
"  argyll: Greame Gill web.access.net.au/argyll/argyllcms.html<br<br><br>\n");
  html << " \
<br>\n";
  html << "\n\n<br>\n</body></html>\n";
  DBG_PROG_ENDE
  return html.str();
}

std::string
getLinksText()
{
  DBG_PROG_START
  std::stringstream text;
  text << _( "Project" ) <<"     : "<< "http://www.behrmann.name/index.php?option=com_content&task=view&id=32&Itemid=1" << "\n";
  text << _( "Documentation" )<<": "<< "http://www.oyranos.org/wiki/index.php?title=ICC_Examin";

  DBG_PROG_ENDE
  return text.str();
}



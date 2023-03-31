////////////////////////////////////////////////////////////////////////////
// Name:        iohumdrum.cpp
// Author:      Craig Stuart Sapp
// Created:     06/06/2016
// Copyright (c) Authors and others. All rights reserved.
////////////////////////////////////////////////////////////////////////////
//
// References:
//    http://humlib.humdrum.org
//    http://music-encoding.org/support/tutorials/mei-1st
//    http://music-encoding.org/support/guidelines
//    http://music-encoding.org/documentation/3.0.0/elements
//    http://music-encoding.org/documentation/3.0.0/atts
//    http://pugixml.org/docs/manual.html
//

#include "iohumdrum.h"
#include "iomei.h"

using namespace std;
#include <iostream>

//----------------------------------------------------------------------------

#ifndef NO_HUMDRUM_SUPPORT

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <locale>
#include <regex>
#include <sstream>
#include <tuple>
#include <vector>

#endif /* NO_HUMDRUM_SUPPORT */

//----------------------------------------------------------------------------

#include "doc.h"

#ifndef NO_HUMDRUM_SUPPORT

#include "accid.h"
#include "app.h"
#include "arpeg.h"
#include "artic.h"
#include "att.h"
#include "barline.h"
#include "beam.h"
#include "beamspan.h"
#include "beatrpt.h"
#include "bracketspan.h"
#include "breath.h"
#include "btrem.h"
#include "choice.h"
#include "chord.h"
#include "custos.h"
#include "dir.h"
#include "dot.h"
#include "dynam.h"
#include "editorial.h"
#include "ending.h"
#include "expansion.h"
#include "f.h"
#include "fb.h"
#include "fermata.h"
#include "fig.h"
#include "ftrem.h"
#include "gliss.h"
#include "hairpin.h"
#include "halfmrpt.h"
#include "harm.h"
#include "instrdef.h"
#include "iomei.h"
#include "keyaccid.h"
#include "keysig.h"
#include "label.h"
#include "labelabbr.h"
#include "layer.h"
#include "lb.h"
#include "lem.h"
#include "ligature.h"
#include "mdiv.h"
#include "measure.h"
#include "mordent.h"
#include "mrest.h"
#include "mrpt.h"
#include "multirest.h"
#include "note.h"
#include "num.h"
#include "octave.h"
#include "orig.h"
#include "page.h"
#include "pb.h"
#include "pedal.h"
#include "pghead.h"
#include "plica.h"
#include "rdg.h"
#include "reg.h"
#include "reh.h"
#include "rend.h"
#include "rest.h"
#include "sb.h"
#include "score.h"
#include "section.h"
#include "slur.h"
#include "space.h"
#include "staff.h"
#include "staffdef.h"
#include "staffgrp.h"
#include "supplied.h"
#include "syl.h"
#include "system.h"
#include "tempo.h"
#include "text.h"
#include "tie.h"
#include "trill.h"
#include "tuplet.h"
#include "turn.h"
#include "verse.h"
#include "vrv.h"
// #include "attcomparison.h"

#endif /* NO_HUMDRUM_SUPPORT */

namespace vrv {

#ifndef NO_HUMDRUM_SUPPORT

#define VGRP_DYNAM_DEFAULT 100
#define VGRP_PEDAL_DEFAULT 200
#define MAXCOLORSUBTRACK 30

//----------------------------------------------------------------------------
// namespace for local IoHumdrum classes
//----------------------------------------------------------------------------

namespace humaux {

    /////////////////////////////////////////////////////////////////////

    HumdrumTie::HumdrumTie()
    {
        clear();
    }

    HumdrumTie::HumdrumTie(const HumdrumTie &anothertie)
    {
        m_starttoken = anothertie.m_starttoken;
        m_endtoken = anothertie.m_endtoken;
        m_starttime = anothertie.m_starttime;
        m_endtime = anothertie.m_endtime;
        m_inserted = anothertie.m_inserted;
        m_above = anothertie.m_above;
        m_below = anothertie.m_below;
        m_startid = anothertie.m_startid;
        m_endid = anothertie.m_endid;
        m_startmeasure = anothertie.m_startmeasure;
        m_endmeasure = anothertie.m_endmeasure;
        m_pitch = anothertie.m_pitch;
        m_layer = anothertie.m_layer;
        m_starttokenpointer = anothertie.m_starttokenpointer;
        m_subindex = anothertie.m_subindex;
        m_meter_top = anothertie.m_meter_top;
        m_meter_bottom = anothertie.m_meter_bottom;
    }

    HumdrumTie::~HumdrumTie()
    {
        clear();
    }

    HumdrumTie &HumdrumTie::operator=(const HumdrumTie &anothertie)
    {
        if (this == &anothertie) {
            return *this;
        }
        m_starttoken = anothertie.m_starttoken;
        m_endtoken = anothertie.m_endtoken;
        m_starttime = anothertie.m_starttime;
        m_endtime = anothertie.m_endtime;
        m_inserted = anothertie.m_inserted;
        m_above = anothertie.m_above;
        m_below = anothertie.m_below;
        m_startid = anothertie.m_startid;
        m_endid = anothertie.m_endid;
        m_startmeasure = anothertie.m_startmeasure;
        m_endmeasure = anothertie.m_endmeasure;
        m_pitch = anothertie.m_pitch;
        m_layer = anothertie.m_layer;
        m_starttokenpointer = anothertie.m_starttokenpointer;
        m_subindex = anothertie.m_subindex;
        m_meter_top = anothertie.m_meter_top;
        m_meter_bottom = anothertie.m_meter_bottom;
        return *this;
    }

    void HumdrumTie::clear()
    {
        m_endmeasure = m_startmeasure = NULL;
        m_inserted = false;
        m_above = false;
        m_below = false;
        m_pitch = 0;
        m_layer = -1;
        m_startid.clear();
        m_endid.clear();
        m_starttokenpointer = NULL;
        m_starttoken = "";
        m_subindex = -1;
        m_meter_top = 4;
        m_meter_bottom = 4;
    }

    void HumdrumTie::setMeterTop(int metertop)
    {
        m_meter_top = metertop;
    }

    void HumdrumTie::setMeterBottom(hum::HumNum meterbot)
    {
        m_meter_bottom = meterbot;
    }

    int HumdrumTie::getMeterTop()
    {
        return m_meter_top;
    }

    hum::HumNum HumdrumTie::getMeterBottom()
    {
        return m_meter_bottom;
    }

    void HumdrumTie::setTieAbove()
    {
        m_above = true;
    }

    void HumdrumTie::setTieBelow()
    {
        m_below = true;
    }

    Tie *HumdrumTie::insertTieIntoDom()
    {
        if (m_inserted) {
            // don't insert again
            return NULL;
        }
        if ((m_startmeasure == NULL) && (m_endmeasure == NULL)) {
            // What are you trying to do?
            return NULL;
        }
        if (m_startmeasure == NULL) {
            // This is a tie with no start.  Don't know what to do with this
            // for now (but is possible due to repeated music).
            return NULL;
        }
        if (m_endmeasure == NULL) {
            // This is a tie with no end.  Don't know what to do with this
            // for now (but is possible due to repeated music).
            return NULL;
        }

        vrv::Tie *tie = new vrv::Tie();
        tie->SetStartid("#" + m_startid);
        tie->SetEndid("#" + m_endid);

        if (m_above) {
            tie->SetCurvedir(curvature_CURVEDIR_above);
        }
        if (m_below) {
            tie->SetCurvedir(curvature_CURVEDIR_below);
        }

        bool samemeasure = false;
        if (m_startmeasure == m_endmeasure) {
            samemeasure = true;
        }

        if (samemeasure) {
            m_startmeasure->AddChild(tie);
            m_inserted = true;
        }
        else {
            // The tie starts in one measure and goes to another.
            // Probably handled the same as when in the same measure.
            m_startmeasure->AddChild(tie);
            m_inserted = true;
        }

        return tie;
    }

    void HumdrumTie::setStart(const std::string &id, Measure *starting, int layer, const std::string &token, int pitch,
        hum::HumNum starttime, hum::HumNum endtime, int subindex, hum::HTp starttok, int metertop, hum::HumNum meterbot)
    {
        m_starttoken = token;
        m_starttime = starttime;
        m_endtime = endtime;
        m_pitch = pitch;
        m_layer = layer;
        m_startmeasure = starting;
        m_startid = id;
        m_subindex = subindex;
        m_starttokenpointer = starttok;
        m_meter_top = metertop;
        m_meter_bottom = meterbot;
    }

    void HumdrumTie::setEnd(const std::string &id, Measure *ending, int layer, const std::string &token, int pitch,
        hum::HumNum starttime, hum::HumNum endtime, int subindex, hum::HTp starttok, int metertop, hum::HumNum meterbot)
    {
        m_endid = id;
        m_layer = layer;
        m_endtoken = token;
        m_endmeasure = ending;
        m_pitch = pitch;
        m_starttime = starttime;
        m_endtime = endtime;
        m_subindex = subindex;
        m_starttokenpointer = starttok; // maybe create m_endtokenpointer for tie ends
        m_meter_top = metertop;
        m_meter_bottom = meterbot;
    }

    hum::HTp HumdrumTie::getEndToken()
    {
        return m_starttokenpointer;
    }

    void HumdrumTie::setEndId(const std::string &id)
    {
        m_endid = id;
    }

    std::string HumdrumTie::getEndId()
    {
        return m_endid;
    }

    Tie *HumdrumTie::setEndAndInsert(const std::string &id, Measure *ending, int layer, const std::string &token,
        int pitch, hum::HumNum starttime, hum::HumNum endtime, int subindex, hum::HTp starttok, int metertop,
        hum::HumNum meterbot)
    {
        setEnd(id, ending, layer, token, pitch, starttime, endtime, subindex, starttok, metertop, meterbot);
        return insertTieIntoDom();
    }

    bool HumdrumTie::isInserted()
    {
        return m_inserted;
    }

    int HumdrumTie::getPitch()
    {
        return m_pitch;
    }

    int HumdrumTie::getLayer()
    {
        return m_layer;
    }

    hum::HumNum HumdrumTie::getStartTime()
    {
        return m_starttime;
    }

    hum::HumNum HumdrumTie::getMeterUnit()
    {
        return m_meter_bottom;
    }

    Measure *HumdrumTie::getStartMeasure()
    {
        return m_startmeasure;
    }

    Measure *HumdrumTie::getEndMeasure()
    {
        return m_endmeasure;
    }

    void HumdrumTie::setEndMeasure(Measure *measure)
    {
        m_endmeasure = measure;
    }

    hum::HumNum HumdrumTie::getEndTime()
    {
        return m_endtime;
    }

    hum::HumNum HumdrumTie::getDuration()
    {
        return m_endtime - m_starttime;
    }

    std::string HumdrumTie::getStartToken()
    {
        return m_starttoken;
    }

    hum::HTp HumdrumTie::getStartTokenPointer()
    {
        return m_starttokenpointer;
    }

    int HumdrumTie::getStartSubindex()
    {
        return m_subindex;
    }

    int HumdrumTie::getEndSubindex()
    {
        return m_subindex;
    }

    /////////////////////////////////////////////////////////////////////

    StaffStateVariables::StaffStateVariables()
    {
        cue_size.resize(100);
        stem_type.resize(100);
        stem_visible.resize(100);
        clear();
    }

    StaffStateVariables::~StaffStateVariables()
    {
        clear();
    }

    void StaffStateVariables::clear()
    {
        verse = false;
        suppress_tuplet_number = false;
        suppress_tuplet_bracket = false;
        suppress_articulations = false;
        tremolo = false;
        pedal = false;
        righthalfstem = false;

        ottavanotestart = ottavanoteend = NULL;
        ottavaendtimestamp = 0;
        ottavameasure = NULL;

        ottavadownnotestart = ottavadownnoteend = NULL;
        ottavadownendtimestamp = 0;
        ottavadownmeasure = NULL;

        ottava2notestart = ottava2noteend = NULL;
        ottava2endtimestamp = 0;
        ottava2measure = NULL;

        ottava2downnotestart = ottava2downnoteend = NULL;
        ottava2downendtimestamp = 0;
        ottava2downmeasure = NULL;

        acclev = 1;
        last_clef = "";

        tiestarts.clear();
        tieends.clear();
        meter_bottom = 4;
        meter_top = 4;

        maximodus = 0;
        modus = 0;
        tempus = 0;
        prolatio = 0;

        auto_custos = false;
        suppress_manual_custos = false;

        verse_labels.clear();
        verse_abbr_labels.clear();

        std::fill(cue_size.begin(), cue_size.end(), false);
        std::fill(stem_type.begin(), stem_type.end(), 'X');
        std::fill(stem_visible.begin(), stem_visible.end(), true);

        mensuration_type = 0;

        join = false;
        glissStarts.clear();
    }

    ostream &StaffStateVariables::print(ostream &out, const std::string &prefix)
    {
        out << prefix << "ADDRESS ==================  " << (long long)this << endl;
        out << prefix << "verse                    =  " << verse << endl;
        out << prefix << "suppress_tuplet_number   =  " << suppress_tuplet_number << endl;
        out << prefix << "suppress_tuplet_bracket  =  " << suppress_tuplet_bracket << endl;
        out << prefix << "suppress_articulations   =  " << suppress_articulations << endl;
        out << prefix << "tremolo                  =  " << tremolo << endl;
        // std::vector<bool> cue_size;
        // std::vector<char> stem_type;
        // std::vector<char> stem_visible;
        out << prefix << "ligature_recta           =  " << ligature_recta << endl;
        out << prefix << "ligature_obliqua         =  " << ligature_obliqua << endl;
        out << prefix << "last_clef                =  " << last_clef << endl;
        out << prefix << "acclev                   =  " << acclev << endl;
        out << prefix << "righthalfstem            =  " << righthalfstem << endl;
        // Note *ottavanotestart;
        // Note *ottavanoteend;
        out << prefix << "ottavaendtimestamp       =  " << ottavaendtimestamp << endl;
        // Measure *ottavameasure;
        // Note *ottavadownnotestart;
        // Note *ottavadownnoteend;
        out << prefix << "ottavadownendtimestamp   =  " << ottavadownendtimestamp << endl;
        // Measure *ottavadownmeasure;
        // Note *ottava2notestart;
        // Note *ottava2noteend;
        out << prefix << "ottava2endtimestamp      =  " << ottava2endtimestamp << endl;
        // Measure *ottava2measure;
        // Note *ottava2downnotestart;
        // Note *ottava2downnoteend;
        out << prefix << "ottava2downendtimestamp  =  " << ottava2downendtimestamp << endl;
        // Measure *ottava2downmeasure;
        out << prefix << "meter_top                =  " << meter_top << endl;
        out << prefix << "meter_bottom             =  " << meter_bottom << endl;
        // std::list<humaux::HumdrumTie> ties;
        out << prefix << "m_dynampos               =  " << m_dynampos << endl;
        out << prefix << "m_dynamstaffadj          =  " << m_dynamstaffadj << endl;
        out << prefix << "m_dynamposdefined        =  " << m_dynamposdefined << endl;
        out << prefix << "auto_custos              =  " << auto_custos << endl;
        out << prefix << "suppress_manual_custos   =  " << suppress_manual_custos << endl;
        out << prefix << "mensuration_type         =  " << mensuration_type << endl;
        out << prefix << "join                     =  " << join << endl;

        return out;
    }

} // end namespace humaux

#endif /* NO_HUMDRUM_SUPPORT */

//----------------------------------------------------------------------------
// HumdrumInput
//----------------------------------------------------------------------------

//////////////////////////////
//
// HumdrumInput::HumdrumInput -- Constructor.
//

HumdrumInput::HumdrumInput(Doc *doc) : Input(doc)
{

#ifndef NO_HUMDRUM_SUPPORT
    m_placement.resize(1000);
    std::fill(m_placement.begin(), m_placement.end(), 0);
    m_reverse.resize(1000);
    std::fill(m_reverse.begin(), m_reverse.end(), 0);
    m_absolute.resize(1000);
    std::fill(m_absolute.begin(), m_absolute.end(), 0);
    m_slash.resize(1000);
    std::fill(m_slash.begin(), m_slash.end(), 1);
#endif /* NO_HUMDRUM_SUPPORT */
}

//////////////////////////////
//
// HumdrumInput::~HumdrumInput -- Deconstructor.
//

HumdrumInput::~HumdrumInput()
{

#ifndef NO_HUMDRUM_SUPPORT
    clear();
#endif /* NO_HUMDRUM_SUPPORT */
}

//////////////////////////////
//
// HumdrumInput::ImportString -- Read a Humdrum file set from a text string.
//

bool HumdrumInput::Import(const std::string &content)
{

#ifndef NO_HUMDRUM_SUPPORT
    try {
        m_doc->Reset();

        // Auto-detect CSV Humdrum file. Maybe later move to the humlib parser.
        std::string exinterp;
        bool found = false;
        int comma = 0;
        int tab = 0;
        for (int i = 0; i < (int)content.size() - 3; ++i) {
            if (((content[i] == '\n') || (content[i] == 0x0d)) && (content[i + 1] == '*') && (content[i + 2] == '*')) {
                found = true;
                i += 2;
                exinterp = "**";
                continue;
            }
            else if ((i == 0) && (content[i] == '*') && (content[i + 1] == '*')) {
                found = true;
                i += 2;
                exinterp = "**";
                continue;
            }

            if (!found) {
                continue;
            }
            if (content[i] == 0x0a) {
                break;
            }
            exinterp.push_back(content[i]);
            if (content[i] == '\t') {
                tab++;
            }
            if (content[i] == ',') {
                comma++;
            }
        }

        bool result;
        if (comma <= tab) {
            result = m_infiles.readString(content);
        }
        else {
            result = m_infiles.readStringCsv(content);
        }

        if (!result) {
            return false;
        }

        return convertHumdrum();
    }
    catch (char *str) {
        LogError("%s", str);
        return false;
    }

#else
    return true;
#endif /* NO_HUMDRUM_SUPPORT */
}

///////////////////////////////////////////////////////////////////////////
//
// Protected functions.
//

#ifndef NO_HUMDRUM_SUPPORT

//////////////////////////////
//
// HumdrumInput::GetHumdrumString -- direct Humdrum output before
//    convesion.
//

string HumdrumInput::GetHumdrumString()
{
    std::stringstream tempout;
    for (int i = 0; i < m_infiles.getCount(); ++i) {
        tempout << m_infiles[i];
    }
    return tempout.str();
}

//////////////////////////////
//
// HumdrumInput::convertHumdrum -- Top level method called from ImportFile or
//     ImportString.  Convert a hum::HumdrumFile structure into an MEI
//     structure.  Returns false if there was an error in the conversion
//     process.
//
// Reference:
//     http://music-encoding.org/documentation/2.1.1/cmn
//

bool HumdrumInput::convertHumdrum()
{
    importVerovioOptions(m_doc);

    if (GetOutputFormat() == "humdrum") {
        // Allow for filtering within toolkit.
        return true;
    }
    if (m_infiles.getCount() == 0) {
        return false;
    }

    // Apply Humdrum tools if there are any filters in the file.
    hum::Tool_filter filter;
    for (int i = 0; i < m_infiles.getCount(); ++i) {
        if (m_infiles[i].hasGlobalFilters()) {
            filter.run(m_infiles[i]);
            if (filter.hasHumdrumText()) {
                m_infiles[i].readString(filter.getHumdrumText());
            }
            else {
                // should have auto updated itself in the filter.
            }
        }
    }

    // Apply Humdrum tools to the entire set if they are
    // at the universal level.
    if (m_infiles.hasUniversalFilters()) {
        filter.runUniversal(m_infiles);
        if (filter.hasHumdrumText()) {
            m_infiles.readString(filter.getHumdrumText());
        }
    }

    hum::HumdrumFile &infile = m_infiles[0];

    // Check if a mensural music score should be produced (and ignore **kerns,
    // since only mens or kern can be rendered by verovio at a time).
    m_mens = checkForMens(infile);
    m_globalTempoScaling = getGlobalTempoScaling(infile);

    bool hasScordatura = checkForScordatura(infile);
    if (hasScordatura) {
        hum::Tool_scordatura scordatura;
        std::vector<std::string> argv;
        argv.push_back("scordatura"); // name of program (placeholder)
        argv.push_back("-w"); // transpose to written pitch
        scordatura.process(argv);
        scordatura.run(infile);
    }

    m_multirest = analyzeMultiRest(infile);
    m_breaks = analyzeBreaks(infile);
    analyzeVerseColor(infile);

    infile.analyzeSlurs();
    infile.analyzeBeams();
    infile.analyzePhrasings();
    infile.analyzeKernTies();
    // infile.analyzeKernStemLengths();
    infile.analyzeRestPositions();
    infile.analyzeKernAccidentals();
    infile.analyzeTextRepetition();
    parseSignifiers(infile);
    if (m_signifiers.terminallong) {
        hideTerminalBarlines(infile);
    }
    if (m_signifiers.terminalbreve) {
        hideTerminalBarlines(infile);
    }
    checkForColorSpine(infile);
    infile.analyzeRScale();
    infile.analyzeCrossStaffStemDirections();
    infile.analyzeBarlines();
    analyzeClefNulls(infile);
    if (infile.hasDifferentBarlines()) {
        adjustMeasureTimings(infile);
    }
    m_spine_color.resize(infile.getMaxTrack() + 1);
    for (int i = 0; i < (int)m_spine_color.size(); ++i) {
        // Hardwire max subtrack count to MAXCOLORSUBTRACK for each spine for now.
        m_spine_color[i].resize(MAXCOLORSUBTRACK);
    }
    initializeSpineColor(infile);
    initializeIgnoreVector(infile);

    bool status = true; // for keeping track of problems in conversion process.

    extractNullInformation(m_nulls, infile);

    int top = -1;
    int bot = -1;
    prepareTimeSigDur(top, bot);
    setupMeiDocument();

    // Create a list of the parts and which spine represents them.
    std::vector<hum::HTp> &staffstarts = m_staffstarts;
    infile.getStaffLikeSpineStartList(staffstarts);
    std::vector<hum::HTp> tempstarts = staffstarts;
    staffstarts.clear();
    std::vector<hum::HTp> tacets;
    for (int i = 0; i < (int)tempstarts.size(); ++i) {
        if (*tempstarts[i] == "**kernyy") {
            continue;
        }
        if (isTacet(tempstarts[i])) {
            tacets.push_back(tempstarts[i]);
            continue;
        }
        staffstarts.push_back(tempstarts[i]);
    }
    if (staffstarts.empty() && !tacets.empty()) {
        staffstarts = tacets;
    }

    m_fbstates.resize(staffstarts.size());
    std::fill(m_fbstates.begin(), m_fbstates.end(), 0);

    m_fbstaff.resize(staffstarts.size());
    std::fill(m_fbstaff.begin(), m_fbstaff.end(), false);

    std::vector<hum::HTp> spinestarts;
    infile.getSpineStartList(spinestarts);
    int staffindex = -1;
    for (auto it : spinestarts) {
        std::string datatype = it->getDataType();
        if (datatype.find("kern") != std::string::npos) {
            staffindex++;
        }
        else if (it->isDataType("**mxhm")) {
            m_harm = true;
        }
        else if (it->isDataType("**mxhm")) {
            m_harm = true;
        }
        else if (it->isDataType("**fing")) {
            m_fing = true;
        }
        else if (it->isDataType("**string")) {
            m_string = true;
        }
        else if (it->isMensLike()) {
            staffindex++;
            m_mens = true;
        }
        else if (it->isDataType("**text")) {
            analyzeTextInterpretation(it);
        }
        else if (it->isDataType("**silbe")) {
            analyzeTextInterpretation(it);
        }
        else if (it->isDataType("**harm")) {
            analyzeHarmInterpretations(it);
            m_harm = true;
        }
        else if (it->isDataType("**deg")) {
            analyzeDegreeInterpretations(it);
            m_degree = true;
        }
        else if (it->isDataType("**degree")) {
            analyzeDegreeInterpretations(it);
            m_degree = true;
        }
        else if (it->isDataType("**rhrm")) { // **recip + **harm
            m_harm = true;
        }
        else if (it->getDataType().compare(0, 7, "**cdata") == 0) {
            m_harm = true;
        }
        else if (it->isDataType("**fb")) {
            m_fb = true;
            if (staffindex >= 0) {
                m_fbstates[staffindex] = -1;
                m_fbstaff[staffindex] = true;
            }
        }
        else if (it->isDataType("**fba")) {
            m_fb = true;
            if (staffindex >= 0) {
                m_fbstates[staffindex] = +1;
                m_fbstaff[staffindex] = true;
            }
        }
        else if (it->isDataType("**Bnum")) {
            // older name
            m_fb = true;
            if (staffindex >= 0) {
                m_fbstates[staffindex] = -1;
                m_fbstaff[staffindex] = true;
            }
        }
    }

    if (staffstarts.size() == 0) {
        // No parts in file, give up.  Perhaps return an error.
        return status;
    }

    // Reverse the order, since top part is last spine.
    reverse(staffstarts.begin(), staffstarts.end());
    calculateReverseKernIndex();

    m_staffstates.resize(staffstarts.size());

    m_transpose.resize(staffstarts.size());
    std::fill(m_transpose.begin(), m_transpose.end(), 0);

    prepareFingerings(infile);
    prepareVerses();
    // ggg problem here: mens calculation after this point or before?
    prepareSections();
    prepareHeaderFooter();
    prepareStaffGroups(top, bot);

    // m_meausreIndex not currently used but might be useful sometime.
    m_measureIndex = 0;
    int line = staffstarts[0]->getLineIndex();
    while (line < infile.getLineCount() - 1 && (line >= 0)) {
        m_measureIndex++;
        status &= convertSystemMeasure(line);
        if (!m_mens) {
            checkForBreak(infile, line);
        }
    }
    if (!m_mens) {
        processHangingTieStarts();
    }

    createHeader();
    // calculateLayout();
    promoteInstrumentAbbreviationsToGroup();
    promoteInstrumentNamesToGroup();

    processHangingTieEnds();

    finalizeDocument(m_doc);

    if (m_debug) {
        cout << GetMeiString();
    }

    // If the document has <pb/> and <sb/> elements you can call:
    //    m_doc->CastOffEncodingDoc();
    // which will cast off the document according to these breaks.
    // Adding <pb/> and <sb/> are done with
    // Pb pb = new Pb();
    // section->AddChild(pb);

    return status;
}

//////////////////////////////
//
// HumdrumInput::analyzeHarmInterpretations --
//   Known interpretations that affect **harm:
//      *[A-Ga-g][#-]*:(dor|phr|lyd|mix|aeo|loc|ion)? == Key designation
//

void HumdrumInput::analyzeHarmInterpretations(hum::HTp starttok)
{
    bool aboveQ = false;
    hum::HTp keydesig = NULL;
    hum::HTp current = starttok;
    while (current) {
        current = current->getNextToken();
        if (!current) {
            break;
        }
        if (current->isData() && !current->isNull()) {
            if (aboveQ) {
                current->setValue("auto", "above", 1);
            }
            else if (keydesig) {
                current->setValue("auto", "meilabel", keydesig->substr(1));
                keydesig = NULL;
            }
        }
        if (!current->isInterpretation()) {
            continue;
        }
        if (*current == "*above") {
            aboveQ = true;
        }
        else if (*current == "*below") {
            aboveQ = false;
        }
        else if (current->isKeyDesignation()) {
            keydesig = current;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeDegreeInterpretations --
//   Known interpretations that affect **harm:
//      *[A-Ga-g][#-]*:(dor|phr|lyd|mix|aeo|loc|ion)? == Key designation
//      *above   == place scale degree above staff
//      *below   == place scale degree below staff (default)
//      *bold    == scale degree displayed in bold
//      *Xbold   == stop displaying scale degrees in bold
//      *circ    == add a circle around the scale degrees
//      *Xcirc   == don't add a circle (default)
//      *hat     == add a hat above the scale degrees
//      *Xhat    == don't add a hat (default)
//      *solf    == display in moveable do
//      *Xsolf   == do not display in moveable do
//

void HumdrumInput::analyzeDegreeInterpretations(hum::HTp starttok)
{
    bool aboveQ = false;
    bool arrowQ = false;
    bool revarrQ = false;
    bool revaccQ = false;
    bool boldQ = false;
    bool boxQ = false;
    bool circleQ = false;
    bool degaccQ = true;
    bool dirQ = true;
    bool hatQ = false;
    bool hideQ = false;
    bool italicQ = false;
    bool minorQ = false;
    bool octaveQ = true;
    bool solfQ = false;
    int circleline = 0;
    int boxline = 0;
    std::string fontsize;
    std::string minmode = "harm";

    hum::HTp keydesig = NULL;
    hum::HTp current = starttok;
    while (current) {
        current = current->getNextToken();
        if (!current) {
            break;
        }
        if (current->isData() && !current->isNull()) {
            if (hideQ) {
                current->setValue("auto", "hidden", 1);
                continue;
            }
            if (aboveQ) {
                // display scale degrees above the staff
                current->setValue("auto", "above", 1);
            }
            if (revarrQ) {
                arrowQ = true;
                revaccQ = false;
                arrowQ = true;
                current->setValue("auto", "arrR", 1);
            }
            if (revaccQ) {
                arrowQ = false;
                revarrQ = false;
                revaccQ = true;
                current->setValue("auto", "accR", 1);
            }
            if (arrowQ || revarrQ) {
                // display chromatic alterations as up/down arrows,
                // but only inform tokens that contain an alteration
                if ((current->find('+') != std::string::npos) || (current->find('-') != std::string::npos)
                    || (current->find('n') != std::string::npos)) {
                    current->setValue("auto", "arrow", 1);
                }
                if ((minmode == "minnat") && (current->find("7") != std::string::npos)) {
                    current->setValue("auto", "arrow", 1);
                }
            }
            if (boldQ) {
                // show scale degree in a bold style
                current->setValue("auto", "bold", 1);
            }
            if (boxQ) {
                if (circleline < boxline) {
                    // show scale degree inside of a box
                    current->setValue("auto", "box", 1);
                }
            }
            if (circleQ) {
                if (boxline < circleline) {
                    // show scale degree inside of a circle
                    current->setValue("auto", "circle", 1);
                }
            }
            if (!degaccQ) {
                // hide chromatic alterations
                current->setValue("auto", "nodegacc", 1);
            }
            if (!dirQ) {
                // do not show the melodic approach direction
                current->setValue("auto", "Xdir", 1);
            }
            if (!fontsize.empty()) {
                // set the size of the scale degree
                current->setValue("auto", "fontsize", fontsize);
            }
            if (hatQ) {
                // display a hat on top of the scale degrees
                current->setValue("auto", "hat", 1);
            }
            if (hideQ) {
                // do not display the scale degree in music notation
                current->setValue("auto", "hide", 1);
            }
            if (italicQ) {
                // show scale degree in an italic style
                current->setValue("auto", "italic", 1);
            }
            if (keydesig) {
                // add key designation as a label on the next
                // scale degree found in the data
                current->setValue("auto", "meilabel", keydesig->substr(1));
                keydesig = NULL;
            }
            if (minorQ) {
                // scale degrees are for a minor key
                current->setValue("auto", "minor", 1);
            }
            if (minorQ && current->find("7") != std::string::npos) {
                if (minmode == "minnat") {
                    current->setValue("auto", "minnat", 1);
                }
            }
            if (!octaveQ) {
                // suppress display of octave information in **degree data
                current->setValue("auto", "Xoctave", 1);
            }
            if (solfQ) {
                // display scale degrees as moveable do
                current->setValue("auto", "solf", 1);
            }
        }
        if (!current->isInterpretation()) {
            continue;
        }
        if (*current == "*above") {
            aboveQ = true;
        }
        else if (*current == "*acc") {
            degaccQ = true;
        }
        else if (*current == "*Xacc") {
            degaccQ = false;
        }
        else if (*current == "*accR") {
            arrowQ = false;
            revaccQ = true;
            revarrQ = false;
        }
        else if (*current == "*arr") {
            arrowQ = true;
        }
        else if (*current == "*Xarr") {
            arrowQ = false;
        }
        else if (*current == "*arrR") {
            arrowQ = true;
            revaccQ = false;
            revarrQ = true;
        }
        else if (*current == "*below") {
            aboveQ = false;
        }
        else if (*current == "*bold") {
            boldQ = true;
        }
        else if (*current == "*Xbold") {
            boldQ = false;
        }
        else if (*current == "*box") {
            boxQ = true;
            boxline = current->getLineIndex();
        }
        else if (*current == "*Xbox") {
            boxQ = false;
            boxline = 0;
        }
        else if (*current == "*circ") {
            circleQ = true;
            circleline = current->getLineIndex();
        }
        else if (*current == "*Xcirc") {
            circleQ = false;
            circleline = 0;
        }
        else if (*current == "*dir") {
            dirQ = true;
        }
        else if (*current == "*Xdir") {
            dirQ = false;
        }
        else if (current->compare(0, 4, "*fs:") == 0) {
            fontsize = current->substr(4);
            if (fontsize == "normal") {
                fontsize.clear();
            }
        }
        else if (*current == "*hat") {
            hatQ = true;
        }
        else if (*current == "*Xhat") {
            hatQ = false;
        }
        else if (*current == "*hide") {
            hideQ = true;
        }
        else if (*current == "*Xhide") {
            hideQ = false;
        }
        else if (*current == "*italic") {
            italicQ = true;
        }
        else if (*current == "*Xitalic") {
            italicQ = false;
        }
        else if (current->isKeyDesignation()) {
            keydesig = current;
            char letter = current->at(1);
            if ((letter == 'X') || (letter == 'x')) {
                // Perhaps a marker for an unknown or no key in the future
                minorQ = false;
            }
            else if (std::islower(letter)) {
                minorQ = true;
            }
            else {
                minorQ = false;
            }
        }
        else if (*current == "*minnat") {
            minmode = "minnat";
        }
        else if (*current == "*minhar") {
            minmode = "minhar";
        }
        else if (*current == "*octave") {
            octaveQ = true;
        }
        else if (*current == "*Xoctave") {
            octaveQ = false;
        }
        else if (*current == "*solf") {
            solfQ = true;
        }
        else if (*current == "*Xsolf") {
            solfQ = false;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeTextInterpretation --  deals with automatic
//     styling of elisions/spaces as well as word extension.
//
// *elision  = Display spaces in **text tokens as elisions (default).
// *Xelision = Do not display spaces as elisions.
// *worex    = Display word extension for melismas after ending of works.
// *Xworex   = Do not display word extension.
//
// *worex/*Xelision override any explicit word extenders ("_" character)
// after ending syllable of words.
//

void HumdrumInput::analyzeTextInterpretation(hum::HTp starttok)
{
    hum::HTp current = starttok;
    hum::HTp lastend = NULL; // last syllable of a word in spine.
    int melismaNoteCount = 0;
    bool elisionQ = true;
    bool foundWorex = false; // used encoded "_" characters for line extension.
    bool worexQ = false;
    hum::HumRegex hre;
    while (current) {
        if (current->isInterpretation()) {
            if (*current == "*elision") {
                elisionQ = true;
            }
            else if (*current == "*Xelision") {
                elisionQ = false;
            }
            if (*current == "*worex") {
                foundWorex = true;
                worexQ = true;
            }
            else if (*current == "*Xworex") {
                foundWorex = true;
                worexQ = false;
            }
        }
        if (!current->isData()) {
            current = current->getNextToken();
            continue;
        }

        // current is a data token at this point.

        if (current->isNull()) {
            // Keep track of any notes that are not
            // attached to a text syllable
            melismaNoteCount += hasParallelNote(current);
            current = current->getNextToken();
            continue;
        }

        // current is some sort of syllable at this point.

        if (foundWorex) {
            // Check for automatic addition or suppresion of word extension lines.
            if (lastend && ((lastend->back() == '_') || (hre.search(lastend, "[^-]$")))) {
                // The last syllable is the end of a word so decide whether or
                // not to add/suppres line extension based on melima note count.
                if (melismaNoteCount) {
                    if (worexQ && !lastend->empty()) {
                        // force a word extender
                        if (lastend->back() != '_') {
                            std::string text = *lastend;
                            text += "_";
                            lastend->setValue("auto", "text", text);
                        }
                    }
                    else {
                        // suppress any word extender
                        if ((!lastend->empty()) && (lastend->back() == '_')) {
                            std::string text = *lastend;
                            text.resize(text.size() - 1);
                            lastend->setValue("auto", "text", text);
                        }
                    }
                }
                melismaNoteCount = 0;
                lastend = NULL;
            }
            if ((current->back() == '_') || (hre.search(current, "[^-]$"))) {
                // This syllable is the end of a word, so reset the melisma count.
                melismaNoteCount = 0;
                lastend = current;
            }
            else {
                lastend = NULL;
            }
        }

        // Check for elision styling.
        if (!elisionQ) {
            if (current->find(' ') == std::string::npos) {
                current = current->getNextToken();
                continue;
            }
            std::string text = *current;
            hre.replaceDestructive(text, "&nbsp;", " ", "g");
            current->setValue("auto", "text", text);
        }

        current = current->getNextToken();
    }

    // If lastend is not NULL, then handle its word extension line state:

    if (foundWorex) {
        // Check for automatic addition or suppresion of word extension lines.
        if (lastend && ((lastend->back() == '_') || (hre.search(lastend, "[^-]$")))) {
            // The last syllable is the end of a word so decide whether or
            // not to add/suppres line extension based on melima note count.
            if (melismaNoteCount) {
                if (worexQ && !lastend->empty()) {
                    // force a word extender
                    if (lastend->back() != '_') {
                        std::string text = *lastend;
                        text += "_";
                        lastend->setValue("auto", "text", text);
                    }
                }
                else {
                    // suppress any word extender
                    if ((!lastend->empty()) && (lastend->back() == '_')) {
                        std::string text = *lastend;
                        text.resize(text.size() - 1);
                        lastend->setValue("auto", "text", text);
                    }
                }
            }
            melismaNoteCount = 0;
            lastend = NULL;
        }
        lastend = NULL;
    }
}

//////////////////////////////
//
// HumdrumInput::hasParallelNote -- Go backwards on the line and count
//   any note attack (or tied note) on the first staff-like spine (track)
//   found to the left.  If there is a spine split in the text and or
//   **kern data, then this algorithm needs to be refined further.
//

int HumdrumInput::hasParallelNote(hum::HTp token)
{
    hum::HTp current = token;
    int track = -1;
    while (current) {
        current = current->getPreviousField();
        if (!current) {
            break;
        }
        if (current->isStaff()) {
            int ctrack = current->getTrack();
            if (track < 0) {
                track = ctrack;
            }
            if (track != ctrack) {
                return 0;
            }
            if (current->isNull()) {
                continue;
            }
            if (current->isNote()) {
                return 1;
            }
        }
    }
    return 0;
}

//////////////////////////////
//
// HumdrumInput::prepareFingerings -- Mark fingerings with explicit
//    above/below placements.
//

void HumdrumInput::prepareFingerings(hum::HumdrumFile &infile)
{
    if (!m_fing) {
        // No fingerings, so nothing to do.
        return;
    }

    std::vector<hum::HTp> fingstarts;
    infile.getSpineStartList(fingstarts, "**fing");
    for (hum::HTp fstart : fingstarts) {
        prepareFingerings(fstart);
    }
}

void HumdrumInput::prepareFingerings(hum::HTp fstart)
{
    //  0 = undefined
    // +1 = above
    // -1 = below
    std::vector<int> states(100, 0);
    hum::HTp current = fstart->getNextToken();
    hum::HTp fcurrent;
    int track;
    int ttrack;
    while (current) {
        if (current->isInterpretation()) {
            fcurrent = current;
            track = fcurrent->getTrack();
            while (fcurrent) {
                ttrack = fcurrent->getTrack();
                if (track != ttrack) {
                    break;
                }
                if (*fcurrent == "*above") {
                    int subtrack = fcurrent->getSubtrack();
                    states.at(subtrack) = +1;
                }
                else if (*fcurrent == "*Xabove") {
                    int subtrack = fcurrent->getSubtrack();
                    states.at(subtrack) = 0;
                }
                else if (*fcurrent == "*below") {
                    int subtrack = fcurrent->getSubtrack();
                    states.at(subtrack) = -1;
                }
                else if (*fcurrent == "*Xbelow") {
                    int subtrack = fcurrent->getSubtrack();
                    states.at(subtrack) = 0;
                }
                fcurrent = fcurrent->getNextFieldToken();
            }
        }
        if (!current->isData()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isNull()) {
            current = current->getNextToken();
            continue;
        }
        fcurrent = current;
        track = fcurrent->getTrack();
        while (fcurrent) {
            ttrack = fcurrent->getTrack();
            if (track != ttrack) {
                break;
            }
            if (fcurrent->isNull()) {
                fcurrent = fcurrent->getNextFieldToken();
                continue;
            }
            int subtrack = fcurrent->getSubtrack();
            if (states.at(subtrack) != 0) {
                if (states.at(subtrack) > 0) {
                    fcurrent->setValue("auto", "place", "above");
                }
                else if (states.at(subtrack) < 0) {
                    fcurrent->setValue("auto", "place", "below");
                }
            }
            fcurrent = fcurrent->getNextFieldToken();
        }
        current = current->getNextToken();
    }
}

//////////////////////////////
//
// HumdrumInput::checkForMens --
//

bool HumdrumInput::checkForMens(hum::HumdrumFile &infile)
{
    std::vector<hum::HTp> spines;
    infile.getSpineStartList(spines);
    for (int i = 0; i < (int)spines.size(); ++i) {
        if (spines[i]->isMensLike()) {
            return true;
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::checkForBreak -- Search for a linebreak or a pagebreak marker,
//     such as:
//          !!linebreak:
//          !!pagebreak:
// There are also layout parameters for barlines that function as line breaks.
// This one is primarily from MusicXML conversion, and can be removed or converted
// to the layout system as need.  Search for a break message anywhere
// around the barline but before any data is found.
//

void HumdrumInput::checkForBreak(hum::HumdrumFile &infile, int line)
{
    if (line >= infile.getLineCount() - 1) {
        return;
    }
    hum::HumNum timestamp = infile[line].getDurationFromStart();
    hum::HumNum ts2;
    int linebreaki = -1;
    int pagebreaki = -1;

    for (int i = line; i < infile.getLineCount(); ++i) {
        if (infile[i].isData()) {
            break;
        }
        if (!infile[i].isGlobalComment()) {
            continue;
        }
        ts2 = infile[i].getDurationFromStart();
        if (ts2 != timestamp) {
            break;
        }
        hum::HTp token = infile[i].token(0);
        if (token->compare(0, 12, "!!linebreak:") == 0) {
            linebreaki = i;
            break;
        }
        else if (token->compare(0, 12, "!!pagebreak:") == 0) {
            pagebreaki = i;
            break;
        }
    }

    if ((linebreaki == -1) && (pagebreaki == -1)) {
        for (int i = line - 1; i > 0; i--) {
            if (infile[i].isData()) {
                break;
            }
            if (!infile[i].isGlobalComment()) {
                continue;
            }
            ts2 = infile[i].getDurationFromStart();
            if (ts2 != timestamp) {
                break;
            }
            hum::HTp token = infile[i].token(0);
            if (token->compare(0, 12, "!!linebreak:") == 0) {
                linebreaki = i;
                break;
            }
            else if (token->compare(0, 12, "!!pagebreak:") == 0) {
                pagebreaki = i;
                break;
            }
        }
    }

    if ((linebreaki == -1) && (pagebreaki == -1)) {
        return;
    }

    // force pagebreaks to linkebreaks for now (see https://github.com/rism-digital/verovio/issues/2034)
    if (pagebreaki > 0) {
        linebreaki = pagebreaki;
        pagebreaki = -1;
    }

    if (linebreaki > 0) {
        hum::HTp token = infile[linebreaki].token(0);
        Sb *sb = new Sb;
        m_layoutInformation = LAYOUT_ENCODED;
        setLocationId(sb, token);
        m_sections.back()->AddChild(sb);
        // Maybe allow other types of line breaks here, but
        // typically break groups should be done with !LO: system.
        if (token->find("original") != std::string::npos) {
            appendTypeTag(sb, "original");
        }
    }
    else if (pagebreaki > 0) {
        hum::HTp token = infile[pagebreaki].token(0);
        Pb *pb = new Pb;
        m_layoutInformation = LAYOUT_ENCODED;
        setLocationId(pb, token);
        m_sections.back()->AddChild(pb);
        // Maybe allow other types of line breaks here, but
        // typically break groups should be done with !!LO: system.
        if (token->find("original") != std::string::npos) {
            appendTypeTag(pb, "original");
        }
    }
}

//////////////////////////////
//
// HumdrumInput::processHangingTieStarts -- Deal with tie starts that were
//    never matched with tie ends.
//

void HumdrumInput::processHangingTieStarts()
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    for (int i = 0; i < (int)ss.size(); ++i) {
        for (auto &it : ss[i].tiestarts) {
            processHangingTieStart(it);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::processHangingTieEnds --  This function is called after
//    processing the score and there are tie ending that were not matched to
//    a tie start.
//

void HumdrumInput::processHangingTieEnds()
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    for (int i = 0; i < (int)ss.size(); ++i) {
        for (auto &it : ss[i].tieends) {

            // This is a hanging tie for no apparent reason.  Display it, but make
            // it red. L.v. will be handled differently as an ornament.
            hum::HTp token = it.getEndToken();
            if (token != NULL) {
                int subindex = it.getEndSubindex();
                hum::HumNum meterunit = it.getMeterUnit();
                Measure *endmeasure = it.getEndMeasure();
                Tie *tie = tieToPreviousItem(token, subindex, meterunit, endmeasure);
                if (tie != NULL) {
                    appendTypeTag(tie, "hanging");
                    tie->SetColor("red");
                    int track = token->getTrack();
                    std::vector<int> &rkern = m_rkern;
                    int staffindex = rkern[track];
                    int staffnum = staffindex + 1;
                    setStaff(tie, staffnum);
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::processHangingTieStart --
//

void HumdrumInput::processHangingTieStart(humaux::HumdrumTie &tieinfo)
{
    hum::HTp token = tieinfo.getStartTokenPointer();
    int subindex = tieinfo.getStartSubindex();
    Measure *measure = tieinfo.getStartMeasure();
    if (measure == NULL) {
        cerr << "Problem with start measure being NULL" << endl;
        return;
    }
    // int metercount = tieinfo.getMeterTop();
    hum::HumNum meterunit = tieinfo.getMeterBottom();
    hum::HumNum duration = token->getDuration();
    hum::HumNum tobegin = token->getDurationFromStart();
    hum::HumNum endtime = tobegin + duration;
    hum::HumNum scordur = token->getOwner()->getOwner()->getScoreDuration();
    int pitch = tieinfo.getPitch();

    if (scordur == tobegin + duration) {
        // This is a hanging tie the goes off of the end of the music
        Tie *tie = addHangingTieToNextItem(token, subindex, meterunit, measure);
        appendTypeTag(tie, "hanging-terminal");
    }
    else if (atEndingBoundaryEnd(token)) {
        Tie *tie = addHangingTieToNextItem(token, subindex, meterunit, measure);
        appendTypeTag(tie, "hanging-terminal-ending");
    }
    else {
        // Check if there is a tie end from an earlier layer that can be linked.
        std::vector<int> &rkern = m_rkern;
        int track = token->getTrack();
        int staffindex = rkern[track];
        std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
        auto &tieends = ss[staffindex].tieends;
        if (tieends.size() > 0) {
            // try to find a matching tie end from a lower layer number
            auto found = ss[staffindex].tieends.end();
            // search for open tie in current layer
            for (auto it = ss[staffindex].tieends.begin(); it != ss[staffindex].tieends.end(); ++it) {
                if (it->getPitch() != pitch) {
                    continue;
                }
                else if (it->getStartTime() == endtime) {
                    found = it;
                    break;
                }
                // deal with disjunct ties as well
            }

            if (found != tieends.end()) {
                // found a matching tie end from a previous layer, so link the ends.
                tieinfo.setEndId(found->getEndId());
                tieinfo.setEndMeasure(found->getEndMeasure());
                Tie *tie = tieinfo.insertTieIntoDom();
                if (tie) {
                    hum::HTp tiestart = tieinfo.getStartTokenPointer();
                    hum::HTp tieend = found->getEndToken();
                    int sindex = tieinfo.getStartSubindex();
                    int eindex = found->getEndSubindex();
                    setTieLocationId(tie, tiestart, sindex, tieend, eindex);
                }
                ss[staffindex].tieends.erase(found);
                return;
            }
        }

        // This is a hanging tie for no apparent reason.  Display it, but make
        // it red unless it is an l.v. tie.

        if (m_signifiers.terminallong && (token->find(m_signifiers.terminallong) != std::string::npos)) {
            // suppress hanging tie (because it was removed)
        }
        else if (m_signifiers.terminalbreve && (token->find(m_signifiers.terminalbreve) != std::string::npos)) {
            // suppress hanging tie (because it was removed)
        }
        else {
            Tie *tie = addHangingTieToNextItem(token, subindex, meterunit, measure);
            appendTypeTag(tie, "hanging");
            bool allowedToHang = isTieAllowedToHang(token);
            std::string lvstring = token->getLayoutParameter("T", "lv");
            bool lv = ((lvstring != "") && (lvstring != "false"));
            if (lv) {
                tie->SetType("lv");
            }
            else if (!allowedToHang) {
                tie->SetColor("red");
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::isTieAllowedToHang -- True if there is a *rep or *> marker
//     before the next non-null data.
//

bool HumdrumInput::isTieAllowedToHang(hum::HTp token)
{
    // Allowed to hang if an l.v. tie
    std::string lv = token->getLayoutParameter("T", "lv");
    if (lv == "true") {
        return true;
    }

    hum::HTp current = token->getNextToken();
    while (current) {
        if (current->isInterpretation()) {
            if (*current == "*rep") {
                return true;
            }
            if (current->compare(0, 2, "*>") == 0) {
                return true;
            }
        }
        else if (current->isData() && !current->isNull()) {
            return false;
        }
        current = current->getNextToken();
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::addHangingTieToNextItem --
//

Tie *HumdrumInput::addHangingTieToNextItem(hum::HTp token, int subindex, hum::HumNum meterunit, Measure *measure)
{
    Tie *tie = new Tie();
    addTieLineStyle(tie, token, subindex);
    measure->AddChild(tie);

    addTieLineStyle(tie, token, subindex);

    std::string lvstring = token->getLayoutParameter("T", "lv");
    hum::HumNum tdur = 0;
    hum::HumRegex hre;
    if (hre.search(lvstring, "\\d")) {
        tdur = hum::Convert::recipToDuration(lvstring);
    }

    hum::HTp trackend = token->getOwner()->getTrackEnd(token->getTrack());
    hum::HTp current = token->getNextToken();
    while (current) {
        if (current->isBarline()) {
            break;
        }
        if (current->isData() && !current->isNull()) {
            break;
        }
        current = current->getNextToken();
    }
    if (current) {
        trackend = current;
    }
    setTieLocationId(tie, token, subindex, trackend, -1);
    std::string startid = getLocationId("note", token);
    if (token->isChord()) {
        int startnumber = subindex + 1;
        if (startnumber > 0) {
            startid += "S" + to_string(startnumber);
        }
    }

    hum::HumNum tstamp;
    if (trackend->isData()) {
        hum::HumNum frombar = trackend->getDurationFromBarline();
        tstamp = frombar;
        tstamp *= meterunit;
        tstamp /= 4;
        tstamp += 1;
    }
    else {
        // attach to barline
        hum::HumNum tobar = token->getDurationToBarline();
        hum::HumNum frombar = token->getDurationFromBarline();
        tstamp = tobar + frombar;
        tstamp *= meterunit;
        tstamp /= 4;
        tstamp += 1;
    }

    tie->SetStartid("#" + startid);

    if (tdur == 0) {
        pair<int, double> ts2(0, tstamp.getFloat());
        tie->SetTstamp2(ts2); // attach start to beginning of measure
    }
    else {
        // attach to end time of lv parameters
        int measures = getMeasureDifference(token, meterunit, tdur, tstamp);
        pair<int, double> ts2(measures, tstamp.getFloat());
        tie->SetTstamp2(ts2);
    }

    int track = token->getTrack();
    std::vector<int> &rkern = m_rkern;
    int staffindex = rkern[track];
    int staffnum = staffindex + 1;
    setStaff(tie, staffnum);

    return tie;
}

//////////////////////////////
//
// HumdrumInput::processHangingTieEnd --
//

void HumdrumInput::processHangingTieEnd(
    Note *note, hum::HTp token, const std::string &tstring, int subindex, hum::HumNum meterunit)
{
    Tie *tie = NULL;
    hum::HumNum position = token->getDurationFromStart();
    if (position == 0) {
        // Hanging tie at start of music.
        tie = tieToPreviousItem(token, subindex, meterunit);
        appendTypeTag(tie, "hanging-initial");
    }
    else if (atEndingBoundaryStart(token)) {
        // The note is at the start of a secondary ending, and
        // is not tied to the previous note, but a note in a previous
        // measure before the first ending.  Also need to force
        // a tie split across ending boundaries (currently they will
        // automatically merge).
        tie = tieToPreviousItem(token, subindex, meterunit);
        appendTypeTag(tie, "hanging-initial-ending");
    }
    else {
        // Store for later processing to link to a tie in a higher
        // layer number, or to identify as a hanging tie.
        std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
        int track = token->getTrack();
        int staffindex = m_rkern[track];
        int cl = m_currentlayer;
        int metertop = ss[staffindex].meter_top;
        hum::HumNum meterbot = ss[staffindex].meter_bottom;
        std::string noteid = note->GetID();
        int pitch = hum::Convert::kernToMidiNoteNumber(tstring);

        hum::HumNum starttime = token->getDurationFromStart();
        hum::HumNum duration = hum::Convert::recipToDuration(tstring);
        hum::HumNum endtime = starttime + duration;
        ss[staffindex].tieends.emplace_back();
        ss[staffindex].tieends.back().setEnd(
            noteid, m_measure, cl, tstring, pitch, starttime, endtime, subindex, token, metertop, meterbot);
    }

    if (tie != NULL) {
        int track = token->getTrack();
        std::vector<int> &rkern = m_rkern;
        int staffindex = rkern[track];
        int staffnum = staffindex + 1;
        setStaff(tie, staffnum);
    }
}

//////////////////////////////
//
// HumdrumInput::atEndingBoundaryStart -- Return true if a token is in a
//   different ending section that the previous note. Split spines
//   should mostly be accounted for, but maybe not corner cases.
//

bool HumdrumInput::atEndingBoundaryStart(hum::HTp token)
{
    hum::HTp current = token->getPreviousToken();
    while (current) {
        if (current->isData() && !current->isNull()) {
            break;
        }
        current = current->getPreviousToken();
    }
    if (!current) {
        return false;
    }
    int line1 = current->getLineIndex();
    int line2 = token->getLineIndex();
    hum::HTp label1 = m_sectionlabels[line1];
    hum::HTp label2 = m_sectionlabels[line2];
    if (label1 == label2) {
        return false;
    }
    if (label1 == NULL) {
        return false;
    }
    if (label2 == NULL) {
        return false;
    }
    hum::HumRegex hre;
    int number1 = 0;
    int number2 = 0;
    if (hre.search(label1, "(\\d+)$")) {
        number1 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (hre.search(label2, "(\\d+)$")) {
        number2 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (number1 == number2) {
        return false;
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::atEndingBoundaryEnd -- Return true if a token is in a
//   different ending section that the next note.
//

bool HumdrumInput::atEndingBoundaryEnd(hum::HTp token)
{
    hum::HTp current = token->getNextToken();
    while (current) {
        if (current->isData() && !current->isNull()) {
            break;
        }
        current = current->getNextToken();
    }
    if (!current) {
        return false;
    }
    int line1 = current->getLineIndex();
    int line2 = token->getLineIndex();
    hum::HTp label1 = m_sectionlabels[line1];
    hum::HTp label2 = m_sectionlabels[line2];
    if (label1 == label2) {
        return false;
    }
    if (label1 == NULL) {
        return false;
    }
    if (label2 == NULL) {
        return false;
    }
    hum::HumRegex hre;
    int number1 = 0;
    int number2 = 0;
    if (hre.search(label1, "(\\d+)$")) {
        number1 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (hre.search(label2, "(\\d+)$")) {
        number2 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (number1 == number2) {
        return false;
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::inDifferentEndings --
//

bool HumdrumInput::inDifferentEndings(hum::HTp token1, hum::HTp token2)
{
    int line1 = token1->getLineIndex();
    int line2 = token2->getLineIndex();
    hum::HTp label1 = m_sectionlabels[line1];
    hum::HTp label2 = m_sectionlabels[line2];
    if (label1 == label2) {
        return false;
    }
    if (label1 == NULL) {
        return false;
    }
    if (label2 == NULL) {
        return false;
    }
    hum::HumRegex hre;
    int number1 = 0;
    int number2 = 0;
    if (hre.search(label1, "(\\d+)$")) {
        number1 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (hre.search(label2, "(\\d+)$")) {
        number2 = hre.getMatchInt(1);
    }
    else {
        return false;
    }
    if (number1 == number2) {
        return false;
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::adjustMeasureTimings --
//

void HumdrumInput::adjustMeasureTimings(hum::HumdrumFile &infile)
{
    hum::HumNum barstart;
    hum::HumNum duration;
    hum::HumNum difference;
    barstart = 0;

    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (infile[i].isBarline()) {
            if (infile[i].allSameBarlineStyle()) {
                // restart barline timestamps for regular barline
                barstart = infile[i].getDurationFromStart();
            }
            else if (infile[i].hasDataStraddle()) {
                // the barlines are not all of the same style
                // and there is at least one staff where there is a duration
                // item (note or rest) that crosses over a barline.  The
                // adjacent measures will be merged into a larger single measure
                // in such cases. (i.e., not resetting barline durations).
            }
            else {
                // The barlines are different, but there is no straddling
                // note or rest.  This will be treated as a regular measure
                // that is invisible at the system level, and with individual
                // barlines of the given style for each staff.
                barstart = infile[i].getDurationFromStart();
            }
        }
        duration = infile[i].getDurationFromStart();
        difference = duration - barstart;
        infile[i].setDurationFromBarline(difference);
    }
    // could also adjust durationToBarline as well.
}

//////////////////////////////
//
// HumdrumInput::checkForScordatura --
//

bool HumdrumInput::checkForScordatura(hum::HumdrumFile &infile)
{
    hum::HumRegex hre;
    bool status = false;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isReference()) {
            continue;
        }
        hum::HTp reference = infile[i].token(0);

        // scordatura markers
        if (hre.search(
                reference, "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s]+)\\s*=.*scordatura\\s*=\\s*[\"']?ITrd(-?\\d)c(-?\\d)")) {
            std::string marker = hre.getMatch(1);
            int diatonic = hre.getMatchInt(2);
            int chromatic = hre.getMatchInt(3);
            if (diatonic == 0 && chromatic == 0) {
                // no transposition needed
                continue;
            }
            bool found = 0;
            // don't allow redundant markers:
            for (int j = 0; j < (int)m_scordatura_marker.size(); j++) {
                if (marker == m_scordatura_marker[j]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                m_scordatura_marker.push_back(marker);
                hum::HumTransposer *transposer = new hum::HumTransposer();
                // The score will be converted to written format, so need to reverse
                // the transposition to get back to the sounding note:
                transposer->setTranspositionDC(-diatonic, -chromatic);
                m_scordatura_transposition.push_back(transposer);
                status = true;
            }
        }
    }
    return status;
}

//////////////////////////////
//
// HumdrumInput::initializeIgnoreVector -- Mark areas of the input file that
//     should not be converted into
//

void HumdrumInput::initializeIgnoreVector(hum::HumdrumFile &infile)
{
    m_ignore.resize(infile.getLineCount());
    int state = false;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        m_ignore[i] = state;
        if (!infile[i].isGlobalComment()) {
            continue;
        }
        if ((std::string)infile[i] == "!!ignore") {
            state = true;
        }
        else if ((std::string)infile[i] == "!!Xignore") {
            state = false;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::extractNullInformation --
//

void HumdrumInput::extractNullInformation(vector<bool> &nulls, hum::HumdrumFile &infile)
{
    nulls.resize(infile.getLineCount());
    // do not analyze in mensural music
    if (m_mens) {
        std::fill(nulls.begin(), nulls.end(), false);
        return;
    }
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isData()) {
            // only keeping track of data null-lines.
            nulls[i] = false;
            continue;
        }
        // Maybe create an HumdrumLine::isAllKernNull() function for this one.
        // But this is probably not a good idea, since there is still a need to
        // place dynamics at fractional positions in relation to the notes sometimes.
        nulls[i] = infile[i].isAllNull();
    }

    m_duradj.resize(infile.getLineCount());
    for (int i = 0; i < (int)m_duradj.size(); ++i) {
        m_duradj[i] = 0;
    }
    hum::HumNum sum = 0;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (infile[i].isBarline()) {
            // Probably suppress null corrections across barlines...
            sum = 0;
            continue;
        }
        if (!infile[i].isData()) {
            continue;
        }
        if (infile[i].getDuration() == 0) {
            continue;
        }
        if (m_nulls[i]) {
            // A null line: collect the duration and move to the next
            // data line that is not null or has 0 duration.
            sum += infile[i].getDuration();
        }
        else if (sum > 0) {
            // A non-null line that precedes some null data lines: store the
            // accumulated duration of the null lines for prespace analysis in
            // fillContentsOfLayer().
            m_duradj[i] = sum;
            sum = 0;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::parseEmbeddedOptions --
//

void HumdrumInput::parseEmbeddedOptions(Doc *doc)
{
    Options *opts = doc->GetOptions();
    if (!opts) {
        return;
    }
    if (m_infiles.getCount() == 0) {
        return;
    }
    hum::HumdrumFile &infile = m_infiles[0];
    hum::HumRegex hre;
    // find the last !!!verovio-parameter-groups: entry in the file
    // (only the last one will be read).
    std::vector<std::string> pgroups;
    for (int i = infile.getLineCount() - 1; i >= 0; i--) {
        if (!infile[i].isReference()) {
            continue;
        }
        std::string key = infile[i].getReferenceKey();
        if (key != "verovio-parameter-group") {
            continue;
        }
        std::string groups = infile[i].getReferenceValue();
        if (groups.empty()) {
            break;
        }
        hre.split(pgroups, groups, "[\\s,]+");
        break;
    }

    std::map<std::string, std::string> inputparameters;
    // Now read through the file searching for verovio parameters
    // that are either unassigned to a group, or is in one of the
    // given groups.
    std::string pkey;
    std::string pvalue;
    std::string value;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isReference()) {
            continue;
        }
        std::string key = infile[i].getReferenceKey();
        if (key == "verovio-parameter-group") {
            continue;
        }
        if (key.compare(0, 7, "verovio") != 0) {
            continue;
        }
        if (key == "verovio") {
            // in the global group, so process always:
            value = infile[i].getReferenceValue();

            std::string pkey;
            std::string pvalue;
            if (hre.search(value, "^\\s*([^\\s]+)\\s+(.*)\\s*$")) {
                pkey = hre.getMatch(1);
                pvalue = hre.getMatch(2);
            }
            else if (hre.search(value, "^\\s*([^\\s]+)\\s*$")) {
                // Empty value which will be interpreted as boolean true.
                pkey = hre.getMatch(1);
                pvalue = "";
            }
            else {
                continue;
            }
            if (value.empty()) {
                cerr << "Warning: value is empty for parameter " << key << endl;
                continue;
            }
            inputparameters[pkey] = pvalue;
        }
        else {
            // check to see if parameter is in desired group
            for (int j = 0; j < (int)pgroups.size(); j++) {
                if (key.compare(8, std::string::npos, pgroups[j]) != 0) {
                    continue;
                }
                value = infile[i].getReferenceValue();

                if (!hre.search(value, "\\s*([^\\s]+)\\s+(.*)\\s*$")) {
                    continue;
                }
                std::string pkey = hre.getMatch(1);
                std::string pvalue = hre.getMatch(2);
                if (value.empty()) {
                    cerr << "Warning: value is empty for parameter " << key << endl;
                    continue;
                }
                inputparameters[pkey] = pvalue;
                break;
            }
        }
    }

    const MapOfStrOptions *optionlist = opts->GetItems();
    for (auto inputoption : inputparameters) {
        auto entry = optionlist->find(inputoption.first);
        if (entry == optionlist->end()) {
            cerr << "Warning: option " << inputoption.first << " is not recognized" << endl;
            continue;
        }

        if (hre.search(inputoption.second, "^([+-]?\\d+\\.?\\d*)$")) {
            double value = hre.getMatchDouble(1);
            entry->second->SetValueDbl(value);
        }
        else if (hre.search(inputoption.second, "^([+-]?\\.\\d+)$")) {
            double value = hre.getMatchDouble(1);
            entry->second->SetValueDbl(value);
        }
        else if (hre.search(inputoption.second, "^\\s*$")) {
            entry->second->SetValueBool(true);
        }
        else {
            entry->second->SetValue(inputoption.second);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::initializeSpineColor -- Look for *color: interpretations before data.
//

void HumdrumInput::initializeSpineColor(hum::HumdrumFile &infile)
{
    hum::HumRegex hre;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (infile[i].isData()) {
            break;
        }
        if (infile[i].isInterpretation()) {
            for (int j = 0; j < infile[i].getFieldCount(); j++) {
                if (hre.search(infile.token(i, j), "^\\*color:(.*)")) {
                    int ctrack = infile.token(i, j)->getTrack();
                    int strack = infile.token(i, j)->getSubtrack();
                    if (strack < MAXCOLORSUBTRACK) {
                        m_spine_color[ctrack][strack] = hre.getMatch(1);
                        if (strack == 1) {
                            m_spine_color[ctrack][0] = m_spine_color[ctrack][1];
                        }
                        else if (strack == 0) {
                            for (int z = 1; z < (int)m_spine_color[ctrack].size(); z++) {
                                m_spine_color[ctrack][z] = m_spine_color[ctrack][0];
                            }
                        }
                    }
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::createHeader --
//
// References:
//      http://music-encoding.org/wp-content/uploads/2015/03/CheatSheetHeader.pdf
//      http://music-encoding.org/support/tutorials/mei-1st/exploring-the-mei-header
//
// <meiHead>
//     <altId>  . . . . . . . . . . Alternate bibliographic identifier.
//     <fileDesc> . . . . . . . . . Full bibliographic description of file.
//        <titleStmt> (required) == Title and responsibility container.
//           <title>  (required) == Title of bibliographic entry.
//           <respStmt>          == Names of those repsonsible for
// intellectual/artistic content.
//        <pubStmt>   (required) == Pub. name, address, date and related info.
//
//        <sourceDesc>           == Sources used to create electronic file.
//           <source n="1">
//              <identifier> <titleStmt> <editionStmt> <putStmt> <physDesc>
//              <seriesStmt> <noteStmt> <history> <langUsage> <key> <tempo>
//              <meter> <perfMedium> <classification> <contents> <relatedItem>
//
//      <encodingDesc>  . . . . . . Relation of file to sources and how it was
// created.
//         <appInfo>? <editorialDecl>?
//
// changed to <workList> in MEI 4.0:
//     <workDesc>  . . . . . . . . Groupling for non-bibliograhpic aspects of
// text.
//        <identifier> <titleStmt> <history> <langUsage> <key> <tempo> <meter>
//        <perfMedium> <notesStmt> <classification> <castList> <incipit>
//        <contents> <relatedItem>
//
//     <revisionDesc>  . . . . . . Alterations to the file.
//        <change>
//

void HumdrumInput::createHeader()
{
    hum::HumdrumFile &infile = m_infiles[0];
    std::vector<hum::HumdrumLine *> references = infile.getReferenceRecords();
    std::vector<std::vector<std::string>> respPeople;
    getRespPeople(respPeople, references);
    pugi::xml_node meiHead = m_doc->m_header.append_child("meiHead");

    // <fileDesc> /////////////
    pugi::xml_node fileDesc = meiHead.append_child("fileDesc");
    pugi::xml_node fileTitle = fileDesc.append_child("titleStmt");

    std::string OTL = getReferenceValue("OTL", references);
    pugi::xml_node title = fileTitle.append_child("title");
    if (!OTL.empty()) {
        title.append_child(pugi::node_pcdata).set_value(unescapeHtmlEntities(OTL).c_str());
    }

    // <pubStmt> /////////////
    pugi::xml_node pubRespStmt;
    pugi::xml_node pubStmt = fileDesc.append_child("pubStmt");
    pugi::xml_document availability;
    for (int i = 0; i < (int)references.size(); ++i) {
        std::string refKey = references[i]->getReferenceKey();
        if (refKey.compare(0, 2, "YE") && refKey.compare(0, 3, "EED") && refKey.compare(0, 3, "PED")) {
            continue;
        }
        else if (refKey.compare(0, 3, "EED") == 0) {
            if (!pubRespStmt) {
                pubRespStmt = pubStmt.prepend_child("respStmt");
            }
            pugi::xml_node editor = pubRespStmt.append_child("persName");
            editor.append_attribute("xml:id") = StringFormat("persname-L%d", references[i]->getLineNumber()).c_str();
            editor.append_attribute("analog") = "humdrum:EED";
            editor.append_attribute("role") = "digital editor";
            editor.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(0, 3, "PED") == 0) {
            if (!pubRespStmt) {
                pubRespStmt = pubStmt.prepend_child("respStmt");
            }
            pugi::xml_node editor = pubRespStmt.append_child("persName");
            editor.append_attribute("xml:id") = StringFormat("persname-L%d", references[i]->getLineNumber()).c_str();
            editor.append_attribute("analog") = "humdrum:PED";
            editor.append_attribute("role") = "source editor";
            editor.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(2, 1, "C") == 0) {
            pugi::xml_node useRestrict = availability.append_child("useRestrict");
            useRestrict.append_attribute("xml:id")
                = StringFormat("userestrict-L%d", references[i]->getLineNumber()).c_str();
            useRestrict.append_attribute("analog") = "humdrum:YEC";
            useRestrict.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(2, 1, "M") == 0) {
            pugi::xml_node useRestrict = availability.append_child("useRestrict");
            useRestrict.append_attribute("xml:id")
                = StringFormat("userestrict-L%d", references[i]->getLineNumber()).c_str();
            useRestrict.append_attribute("analog") = "humdrum:YEM";
            useRestrict.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(2, 1, "N") == 0) {
            pugi::xml_node pubPlace = pubStmt.append_child("pubPlace");
            pubPlace.append_attribute("xml:id") = StringFormat("pubplace-L%d", references[i]->getLineNumber()).c_str();
            pubPlace.append_attribute("analog") = "humdrum:YEN";
            pubPlace.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(2, 1, "P") == 0) {
            pugi::xml_node publisher = pubStmt.append_child("publisher");
            publisher.append_attribute("xml:id")
                = StringFormat("publisher-L%d", references[i]->getLineNumber()).c_str();
            publisher.append_attribute("analog") = "humdrum:YEP";
            publisher.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
        else if (refKey.compare(2, 1, "R") == 0) {
            pugi::xml_node pubDate = pubStmt.append_child("date");
            pubDate.append_attribute("xml:id") = StringFormat("date-L%d", references[i]->getLineNumber()).c_str();
            pubDate.append_attribute("analog") = "humdrum:YER";
            pubDate.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
        }
    }
    if (availability.first_child()) {
        pugi::xml_node copyright = pubStmt.append_child("availability");
        for (pugi::xml_node child = availability.first_child(); child; child = child.next_sibling()) {
            copyright.append_copy(child);
        }
        availability.reset();
    }

    // <encodingDesc> /////////
    pugi::xml_node encodingDesc = meiHead.append_child("encodingDesc");

    // <appInfo> /////////
    pugi::xml_node appInfo = encodingDesc.append_child("appInfo");
    pugi::xml_node application = appInfo.append_child("application");
    application.append_attribute("isodate") = getDateString().c_str();
    application.append_attribute("version") = GetVersion().c_str();
    pugi::xml_node name = application.append_child("name");
    name.append_child(pugi::node_pcdata).set_value("Verovio");
    pugi::xml_node p1 = application.append_child("p");
    p1.append_child(pugi::node_pcdata).set_value("Transcoded from Humdrum");
    // <editorialDecl> /////////
    std::string RNB = getReferenceValue("RNB", references);
    std::string RWG = getReferenceValue("RWG", references);
    if (!RNB.empty() || !RWG.empty()) {
        pugi::xml_node editorialDecl = encodingDesc.append_child("editorialDecl");
        for (int i = 0; i < (int)references.size(); ++i) {
            std::string key = references[i]->getReferenceKey();
            if (key == "RNB") {
                pugi::xml_node note = editorialDecl.append_child("p");
                note.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
                note.append_attribute("label") = "note";
            }
            if (key == "RWG") {
                pugi::xml_node warning = editorialDecl.append_child("p");
                warning.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
                warning.append_attribute("label") = "warning";
            }
        }
    }
    // <projectDesc> /////////
    std::string ENC = getReferenceValue("ENC", references);
    std::string EEV = getReferenceValue("EEV", references);
    if (!ENC.empty() || !EEV.empty()) {
        pugi::xml_node projectDesc = encodingDesc.append_child("projectDesc");
        if (!ENC.empty()) {
            ENC = "Encoded by: " + ENC;
            pugi::xml_node p2 = projectDesc.append_child("p");
            p2.append_child(pugi::node_pcdata).set_value(ENC.c_str());
        }
        if (!EEV.empty()) {
            EEV = "Version: " + EEV;
            pugi::xml_node p3 = projectDesc.append_child("p");
            p3.append_child(pugi::node_pcdata).set_value(EEV.c_str());
        }
    }

    // <sourceDesc> /////////

    // <workDesc> /////////////
    // <workDesc> changed to <workList> in MEI 4.0
    // pugi::xml_node workDesc = meiHead.append_child("workDesc");
    pugi::xml_node workList = meiHead.append_child("workList");
    // pugi::xml_node work = workDesc.append_child("work");
    pugi::xml_node work = workList.append_child("work");

    std::string SCT = getReferenceValue("SCT", references);
    if (!SCT.empty()) {
        pugi::xml_node identifier = work.append_child("identifier");
        identifier.append_attribute("analog") = "humdrum:SCT";
        identifier.append_child(pugi::node_pcdata).set_value(SCT.c_str());
    }
    // <titleStmt> removed in MEI 4.0
    // pugi::xml_node titleStmt = work.append_child("titleStmt");
    // pugi::xml_node titleStmt = work.append_child("titleStmt");
    // insertTitle(titleStmt, references);
    insertTitle(work, references);
    if (respPeople.size() > 0) {
        // insertRespStmt(titleStmt, respPeople);
        // Update for MEI 4.0:
        insertPeople(work, respPeople);
    }
    std::string ODT = getReferenceValue("ODT", references);
    std::string OCY = getReferenceValue("OCY", references);
    std::string OPC = getReferenceValue("OPC", references);
    if (!ODT.empty() || !OCY.empty() || !OPC.empty()) {
        pugi::xml_node creation = work.append_child("creation");
        if (!ODT.empty()) {
            pugi::xml_node date = creation.append_child("date");
            date.append_attribute("analog") = "humdrum:ODT";
            date.append_child(pugi::node_pcdata).set_value(ODT.c_str());
        }
        if (!OCY.empty()) {
            pugi::xml_node country = creation.append_child("geogName");
            country.append_attribute("analog") = "humdrum:OCY";
            country.append_child(pugi::node_pcdata).set_value(OCY.c_str());
        }
        if (!OPC.empty()) {
            pugi::xml_node place = creation.append_child("geogName");
            place.append_attribute("analog") = "humdrum:OPC";
            place.append_child(pugi::node_pcdata).set_value(OPC.c_str());
        }
    }
    std::string HAO = getReferenceValue("HAO", references);
    if (!HAO.empty()) {
        pugi::xml_node history = work.append_child("history");
        history.append_attribute("analog") = "humdrum:HAO";
        for (int i = 0; i < (int)references.size(); ++i) {
            std::string refKey = references[i]->getReferenceKey();
            if (refKey.compare(0, 3, "HAO")) {
                continue;
            }
            else {
                pugi::xml_node histLine = history.append_child("p");
                histLine.append_attribute("xml:id") = StringFormat("p-L%d", references[i]->getLineNumber()).c_str();
                histLine.append_child(pugi::node_pcdata).set_value(references[i]->getReferenceValue().c_str());
            }
        }
    }

    // <extMeta> /////////////
    if (references.size() > 0) {
        insertExtMeta(references);
    }

    // pugi::xml_node creation = work.append_child("creation");
}

//////////////////////////////
//
// HumdrumInput::getReferenceValue --
//

string HumdrumInput::getReferenceValue(const std::string &key, std::vector<hum::HumdrumLine *> &references)
{
    for (int i = 0; i < (int)references.size(); ++i) {
        if (key == references[i]->getReferenceKey()) {
            return references[i]->getReferenceValue();
        }
    }
    std::string empty;
    return empty;
}

//////////////////////////////
//
// HumdrumInput::getDateSting -- Return the current time and date as a std::string.
//

string HumdrumInput::getDateString()
{
    time_t t = time(0); // get time now
    struct tm *now = localtime(&t);
    std::string dateStr = StringFormat("%d-%02d-%02dT%02d:%02d:%02d", now->tm_year + 1900, now->tm_mon + 1,
        now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    return dateStr;
}

//////////////////////////////
//
// HumdrumInput::insertRespStmt -- Print a list of composer and/or lyricist.
//     This is for MEI 3.0 and no longer used, so should eventually be deleted.
//

void HumdrumInput::insertRespStmt(pugi::xml_node &titleStmt, std::vector<std::vector<std::string>> &respPeople)
{
    if (respPeople.size() == 0) {
        return;
    }
    pugi::xml_node respStmt = titleStmt.append_child("respStmt");
    for (int i = 0; i < (int)respPeople.size(); ++i) {
        pugi::xml_node person = respStmt.append_child("persName");
        person.append_attribute("xml:id") = StringFormat("persname-L%s", respPeople[i][3].c_str()).c_str();
        person.append_attribute("analog") = StringFormat("humdrum:%s", respPeople[i][2].c_str()).c_str();
        person.append_attribute("role") = unescapeHtmlEntities(respPeople[i][1]).c_str();
        person.text().set(unescapeHtmlEntities(respPeople[i][0]).c_str());
    }
}

//////////////////////////////
//
// HumdrumInput::insertPeople -- Print a list of composer and/or lyricist.
//    Adapted from HumdrumInput::insertRespStmt().
//    Each entry in respPeople:
//   [0] = Person's name
//   [1] = role:
//          COM = composer
//          COA = attributed composer
//          COS = suspected composer
//          LYR = lyricist composer
//          see definition of getRespPeople() for more roles.
//   [2] = @analog
//   [3] = Line number for xml:id creation
//

void HumdrumInput::insertPeople(pugi::xml_node &work, std::vector<std::vector<std::string>> &respPeople)
{
    if (respPeople.size() == 0) {
        return;
    }
    // pugi::xml_node respStmt = titleStmt.append_child("respStmt");
    bool created = false;
    pugi::xml_node person;
    for (int i = 0; i < (int)respPeople.size(); ++i) {
        created = false;
        if (respPeople[i][1] == "composer") {
            person = work.append_child("composer");
            created = true;
        }
        if (respPeople[i][1] == "attributed composer") {
            person = work.append_child("composer");
            person.append_attribute("cert") = "unknown";
            created = true;
        }
        if (respPeople[i][1] == "suspected composer") {
            person = work.append_child("composer");
            person.append_attribute("cert") = "unknown";
            created = true;
        }
        else if (respPeople[i][1] == "lyricist") {
            person = work.append_child("lyricist");
            created = true;
        }
        else if (respPeople[i][1] == "librettist") {
            person = work.append_child("librettist");
            created = true;
        }

        if (created) {
            person.text().set(unescapeHtmlEntities(respPeople[i][0]).c_str());
            if (!respPeople[i][2].empty()) {
                person.append_attribute("analog") = StringFormat("humdrum:%s", respPeople[i][2].c_str()).c_str();
            }
            person.append_attribute("xml:id") = StringFormat("person-L%s", respPeople[i][3].c_str()).c_str();
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getRespPeople -- Get the respStmnt people, such as the composer
// and/or lyricist.
//
// Roles (4th parameter in addPerson(), is free-form, but should use the roles
// are listed in these two webpages:
//    http://www.loc.gov/marc/relators/relacode.html
//       list of three-letter relator codes
//    http://www.loc.gov/marc/relators/relaterm.html
//       short descriptions of relator codes
//

void HumdrumInput::getRespPeople(
    std::vector<std::vector<std::string>> &respPeople, std::vector<hum::HumdrumLine *> &references)
{

    // precalculate a reference map here to make more O(N) rather than O(N^2)
    addPerson(respPeople, references, "COM", "composer"); // cmp
    addPerson(respPeople, references, "COA", "attributed composer");
    addPerson(respPeople, references, "COS", "suspected composer");
    addPerson(respPeople, references, "LYR", "lyricist"); // lyr
    addPerson(respPeople, references, "LIB", "librettist"); // lbt
    addPerson(respPeople, references, "LAR", "arranger"); // arr
    addPerson(respPeople, references, "LOR", "adapter"); // orchestrator, adp
    addPerson(respPeople, references, "RPN", "producer"); // pro
    addPerson(respPeople, references, "MPN", "performer"); // prf, also: Singer/Instrumentalist
    addPerson(respPeople, references, "MCN", "conductor"); // cnd
    addPerson(respPeople, references, "ODE", "dedicatee"); // dte
    addPerson(respPeople, references, "OCO", "patron"); // commissioner, pat
    addPerson(respPeople, references, "OCL", "collector"); // col
    addPerson(respPeople, references, "PED", "source editor");
    addPerson(respPeople, references, "EED", "digital editor");
    addPerson(respPeople, references, "ENC", "encoder"); // mrk,
    // Markup editor
}

//////////////////////////////
//
// HumdrumInput::addPerson --
//

void HumdrumInput::addPerson(std::vector<std::vector<std::string>> &respPeople,
    std::vector<hum::HumdrumLine *> &references, const std::string &key, const std::string &role)
{
    for (int i = 0; i < (int)references.size(); ++i) {
        if (references[i]->getReferenceKey() == key) {
            respPeople.resize(respPeople.size() + 1);
            respPeople.back().resize(4);
            respPeople.back()[0] = references[i]->getReferenceValue();
            respPeople.back()[1] = role;
            respPeople.back()[2] = key;
            respPeople.back()[3] = std::to_string(references[i]->getLineNumber());
        }
    }
}

//////////////////////////////
//
// HumdrumInput::insertExtMeta -- Print Humdrum reference records as XML into
//     <extMeta> element.
//

void HumdrumInput::insertExtMeta(std::vector<hum::HumdrumLine *> &references)
{
    // for now do not print for **mens data, since timestamps are used
    if (m_mens) {
        return;
    }
    std::stringstream xmldata;
    xmldata << "<extMeta>\n";
    xmldata << "\t<frames xmlns=\"http://www.humdrum.org/ns/humxml\">\n";
    for (int i = 0; i < (int)references.size(); ++i) {
        std::string refKey = references[i]->getReferenceKey();
        // Keep all reference records for round-trip conversions:
        // if (!(refKey.compare(0, 3, "EED") && refKey.compare(0, 2, "HA") && refKey.compare(0, 2, "OT")
        //         && refKey.compare(0, 2, "YE") && refKey.compare(0, 1, "X"))) {
        //     continue;
        // }
        references[i]->printXml(xmldata, 4);
    }
    xmldata << "\t</frames>\n";
    xmldata << "</extMeta>\n";

    pugi::xml_document tmpdoc;
    pugi::xml_parse_result result = tmpdoc.load_string(xmldata.str().c_str());
    if (!result) {
        // some sort of error, so give up;
        cerr << "ExtMeta parse error: " << result.description() << endl;
        cerr << xmldata.str();
        return;
    }

    m_doc->m_header.first_child().append_copy(tmpdoc.document_element());
}

//////////////////////////////
//
// HumdrumInput::insertTitle --
//

void HumdrumInput::insertTitle(pugi::xml_node &work, const std::vector<hum::HumdrumLine *> &references)
{
    std::string key;
    std::string value;
    bool lang;
    bool plang;
    std::string language;

    int titlecount = 0;

    for (int i = 0; i < (int)references.size(); ++i) {
        plang = false;
        lang = false;
        key = references[i]->getReferenceKey();
        if (key.compare(0, 2, "OT") && key.compare(0, 1, "X")) {
            continue;
        }
        value = references[i]->getReferenceValue();
        if (value.empty()) {
            continue;
        }
        auto loc = key.find("@");
        if (loc != std::string::npos) {
            lang = true;
            if (key.find("@@") != std::string::npos) {
                plang = true;
                language = key.substr(loc + 2);
                if (language.empty()) {
                    plang = false;
                    lang = false;
                }
            }
            else {
                language = key.substr(loc + 1);
                if (language.empty()) {
                    plang = false;
                    lang = false;
                }
            }
        }

        for (int j = 0; j < (int)language.size(); ++j) {
            if (language[j] == '-') {
                // don't force to lower case after first dash
                // as BCP 47 country codes are in upper case, and
                // variant codes usually start capitalized.
                break;
            }
            language[j] = std::tolower(language[j]);
        }

        pugi::xml_node title = work.append_child("title");
        titlecount++;
        title.text().set(unescapeHtmlEntities(value).c_str());
        title.append_attribute("xml:id") = StringFormat("title-L%d", references[i]->getLineNumber()).c_str();
        title.append_attribute("analog") = StringFormat("humdrum:%s", key.substr(0, 3).c_str()).c_str();
        if (key.compare(0, 3, "OTL") == 0) {
            if (!lang || plang) {
                title.append_attribute("type") = "main";
            }
            else {
                title.append_attribute("type") = "translated";
            }
            if (lang) {
                title.append_attribute("xml:lang") = language.c_str();
            }
        }
        else if (key.compare(0, 3, "OTA") == 0) {
            title.append_attribute("type") = "alternative";
            if (lang) {
                title.append_attribute("xml:lang") = language.c_str();
            }
        }
        else if (key.compare(0, 3, "OTP") == 0) {
            title.append_attribute("type") = "alternative";
            title.append_attribute("label") = "popular";
            if (lang) {
                title.append_attribute("xml:lang") = language.c_str();
            }
        }
        else {
            title.append_attribute("type") = "translated";
        }
    }

    if (!titlecount) {
        // Put in a required empty <title/> tag:
        work.append_child("title");
    }
}

//////////////////////////////
//
// HumdrumInput::prepareVerses -- Assumes that m_staffstarts has been
//      filled already.
//

void HumdrumInput::prepareVerses()
{
    int i, j;
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    // ss[*].verse should already be set to false.

    std::vector<hum::HTp> &staffstarts = m_staffstarts;

    if (staffstarts.size() == 0) {
        return;
    }

    hum::HumdrumLine &line = *staffstarts[0]->getLine();
    int field;

    for (i = 0; i < (int)staffstarts.size(); ++i) {
        field = staffstarts[i]->getFieldIndex();
        for (j = field + 1; j < line.getFieldCount(); ++j) {
            if (line.token(j)->isKernLike()) {
                break;
            }
            if (line.token(j)->isMensLike()) {
                break;
            }
            else if (line.token(j)->isDataTypeLike("**text")) {
                ss[i].verse = true;
            }
            else if (line.token(j)->isDataTypeLike("**silbe")) {
                ss[i].verse = true;
            }
            else if (line.token(j)->getDataType().compare(0, 7, "**vdata") == 0) {
                ss[i].verse = true;
            }
            else if (line.token(j)->getDataType().compare(0, 8, "**vvdata") == 0) {
                ss[i].verse = true;
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::prepareTimeSigDur -- create a list of the durations of time
//      signatures in the file, indexed by hum::HumdrumFile line number.  Only
//      the first spine in the file is considered.
//

void HumdrumInput::prepareTimeSigDur(int &top, int &bot)
{
    std::vector<hum::HumNum> &sigdurs = m_timesigdurs;
    hum::HumdrumFile &infile = m_infiles[0];
    std::vector<hum::HTp> spinestarts;

    sigdurs.resize(infile.getLineCount());
    std::fill(sigdurs.begin(), sigdurs.end(), -1);

    infile.getKernLikeSpineStartList(spinestarts);
    hum::HTp kernspine = NULL;
    if (spinestarts.size() == 0) {
        infile.getSpineStartList(spinestarts, "**recip");
        if (spinestarts.size() == 0) {
            // no **kern or **recip so give up
            return;
        }
        else {
            kernspine = spinestarts[0];
        }
    }
    else {
        kernspine = spinestarts[0];
    }
    if (kernspine == NULL) {
        return;
    }

    hum::HumNum curdur = -1;
    int bot2;
    int line;

    kernspine = kernspine->getNextToken();
    while (kernspine) {
        line = kernspine->getLineIndex();
        if (!kernspine->isInterpretation()) {
            sigdurs[line] = curdur;
            kernspine = kernspine->getNextToken();
            continue;
        }

        if (sscanf(kernspine->c_str(), "*M%d/%d%%%d", &top, &bot, &bot2) == 3) {
            // deal with triplet-whole note beats later
        }
        else if (sscanf(kernspine->c_str(), "*M%d/%d", &top, &bot) == 2) {
            curdur = top;
            if (bot == 0) { // breve
                curdur *= 2;
            }
            else {
                curdur /= bot;
            }
            curdur *= 4; // convert to quarter note units;
        }
        sigdurs[line] = curdur;
        kernspine = kernspine->getNextToken();
    }

    sigdurs.back() = curdur;
    for (int i = (int)sigdurs.size() - 2; i >= 0; i--) {
        if (infile[i].getDuration() == 0) {
            sigdurs[i] = sigdurs[i + 1];
        }
    }
}

//////////////////////////////
//
// HumdrumInput::calculateReverseKernIndex --
//

void HumdrumInput::calculateReverseKernIndex()
{
    std::vector<int> &rkern = m_rkern;
    hum::HumdrumFile &infile = m_infiles[0];
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;

    rkern.resize(infile.getSpineCount() + 1);
    std::fill(rkern.begin(), rkern.end(), -1);
    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        rkern[staffstarts[i]->getTrack()] = i;
    }
}

//////////////////////////////
//
// HumdrumInput::prepareStaffGroups --  Add information about each part and
//    group by brackets/bar groupings (for initial staff information).
//

void HumdrumInput::prepareStaffGroups(int top, int bot)
{
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;

    if (staffstarts.size() > 0) {
        addMidiTempo(m_doc->GetCurrentScoreDef(), staffstarts[0], top, bot);
    }
    hum::HumRegex hre;
    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        m_staffdef.push_back(new StaffDef());
        setLocationId(m_staffdef.back(), staffstarts[i]);
        if (hre.search(staffstarts[i], "^\\*\\*kern-(.*)")) {
            m_staffdef.back()->SetType(hre.getMatch(1));
        }
        // m_staffgroup->AddChild(m_staffdef.back());
        int staffnumber = i + 1;
        int staffcount = (int)staffstarts.size();
        fillStaffInfo(staffstarts[i], staffnumber, staffcount);
    }

    std::string decoration = getSystemDecoration("system-decoration");

    if (decoration == "") {
        // Set a default decoration style depending on the staff count.
        // If there are two staves, presume that it is for a grand staff
        // and a brace should be displayed.
        if (staffstarts.size() == 2) {
            processStaffDecoration("{(*)}");
        }

        // If there are more than two staves then
        // add a bracket around the staves.
        else if (staffstarts.size() > 2) {
            processStaffDecoration("[*]");
        }

        // If there is one staff, then no extra decoration.
        else if (staffstarts.size() == 1) {
            StaffGrp *sg = new StaffGrp();
            m_doc->GetCurrentScoreDef()->AddChild(sg);
            sg->AddChild(m_staffdef[0]);
        }
        // do something if there is no staff in the score?
    }
    else {
        bool status = processStaffDecoration(decoration);
        if (!status) {
            StaffGrp *sg = new StaffGrp();
            m_doc->GetCurrentScoreDef()->AddChild(sg);
            sg->SetBarThru(BOOLEAN_false);
            // setGroupSymbol(sg, staffGroupingSym_SYMBOL_bracket);
            for (int i = 0; i < (int)m_staffdef.size(); ++i) {
                sg->AddChild(m_staffdef[i]);
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::promoteInstrumentNamesToGroup -- If two staves are in a staff group and only one
//    has an instrument label, or both have identical labels, then move the label to the group
//    level, so that the instrument name/abbreviation is centered between the staves.
//
// scoreDef
//    staffDef
//    staffGrp
//       staffDef
//       staffGrp
//          staffDef
//          staffDef
//

void HumdrumInput::promoteInstrumentNamesToGroup()
{
    ScoreDef *sdf = m_doc->GetCurrentScoreDef();
    int count = sdf->GetChildCount();
    for (int i = 0; i < count; ++i) {
        Object *obj = sdf->GetChild(i);
        std::string name = obj->GetClassName();
        if (name != "StaffGrp") {
            continue;
        }
        StaffGrp *sg = (StaffGrp *)obj;
        promoteInstrumentsForStaffGroup(sg);
    }
}

//////////////////////////////
//
// HumdrumInput::promoteInstrumentsForStaffGroup --
//
//

void HumdrumInput::promoteInstrumentsForStaffGroup(StaffGrp *group)
{
    int count = group->GetChildCount();
    std::vector<std::string> names;
    std::string name;
    std::vector<StaffDef *> sds;
    sds.clear();
    for (int i = 0; i < count; ++i) {
        Object *obj = group->GetChild(i);
        name = obj->GetClassName();
        if (name == "StaffGrp") {
            promoteInstrumentsForStaffGroup((StaffGrp *)obj);
        }
        if (name != "StaffDef") {
            continue;
        }
        StaffDef *sd = (StaffDef *)obj;
        sds.push_back(sd);
        std::string label = getInstrumentName(sd);
        names.push_back(label);
    }
    if (names.size() <= 1) {
        return;
    }
    if (sds.size() != 2) {
        return;
    }
    std::string nonempty = names[0];
    for (int i = 1; i < (int)names.size(); ++i) {
        if (names[i] == "") {
            continue;
        }
        if (nonempty == "") {
            nonempty = names[i];
            continue;
        }
        if (names[i] != nonempty) {
            return;
            break;
        }
    }

    setInstrumentName(group, nonempty);
    for (int i = 0; i < (int)sds.size(); ++i) {
        removeInstrumentName(sds[i]);
    }
}

//////////////////////////////
//
// HumdrumInput::promoteInstrumentAbbreviationsToGroup --
//

void HumdrumInput::promoteInstrumentAbbreviationsToGroup()
{
    ScoreDef *sdf = m_doc->GetCurrentScoreDef();
    int count = sdf->GetChildCount();

    for (int i = 0; i < count; ++i) {
        Object *obj = sdf->GetChild(i);
        std::string name = obj->GetClassName();
        if (name != "StaffGrp") {
            continue;
        }
        StaffGrp *sg = (StaffGrp *)obj;
        promoteInstrumentAbbreviationsForStaffGroup(sg);
    }
}

//////////////////////////////
//
// HumdrumInput::promoteInstrumentAbbreviationsForStaffGroup --
//
//

void HumdrumInput::promoteInstrumentAbbreviationsForStaffGroup(StaffGrp *group)
{
    int count = group->GetChildCount();
    std::vector<std::string> names;
    std::string name;
    std::vector<StaffDef *> sds;
    sds.clear();
    for (int i = 0; i < count; ++i) {
        Object *obj = group->GetChild(i);
        name = obj->GetClassName();
        if (name == "StaffGrp") {
            promoteInstrumentAbbreviationsForStaffGroup((StaffGrp *)obj);
        }
        if (name != "StaffDef") {
            continue;
        }
        StaffDef *sd = (StaffDef *)obj;
        sds.push_back(sd);
        std::string label = getInstrumentAbbreviation(sd);
        names.push_back(label);
    }
    if (names.size() <= 1) {
        return;
    }
    if (sds.size() != 2) {
        return;
    }
    std::string nonempty = names[0];
    for (int i = 1; i < (int)names.size(); ++i) {
        if (names[i] == "") {
            continue;
        }
        if (nonempty == "") {
            nonempty = names[i];
            continue;
        }
        if (names[i] != nonempty) {
            return;
        }
    }

    setInstrumentAbbreviation(group, nonempty, NULL);
    for (int i = 0; i < (int)sds.size(); ++i) {
        if (names.at(i).empty()) {
            continue;
        }
        removeInstrumentAbbreviation(sds[i]);
    }
}

//////////////////////////////
//
// HumdrumInput::removeInstrumentName -- Assuming only one label for staffDef.
//

void HumdrumInput::removeInstrumentName(StaffDef *sd)
{
    Object *obj = sd->FindDescendantByType(ClassId::LABEL);
    if (!obj) {
        return;
    }
    sd->DeleteChild(obj);
}

//////////////////////////////
//
// HumdrumInput::removeInstrumentAbbreviation -- Assuming only one abbreviation for staffDef.
//

void HumdrumInput::removeInstrumentAbbreviation(StaffDef *sd)
{
    Object *obj = sd->FindDescendantByType(ClassId::LABELABBR);
    if (!obj) {
        return;
    }
    sd->DeleteChild(obj);
}

//////////////////////////////
//
// HumdrumInput::getInstrumentName --
//

std::string HumdrumInput::getInstrumentName(StaffDef *sd)
{
    Object *label = sd->FindDescendantByType(ClassId::LABEL);
    if (!label) {
        return "";
    }
    Object *obj = label->FindDescendantByType(ClassId::TEXT);
    if (!obj) {
        return "";
    }
    Text *text = (Text *)obj;
    std::string name = UTF32to8(text->GetText());
    if (name == "    ") {
        name = "";
    }
    return name;
}

//////////////////////////////
//
// HumdrumInput::getInstrumentAbbreviation --
//

std::string HumdrumInput::getInstrumentAbbreviation(StaffDef *sd)
{
    Object *label = sd->FindDescendantByType(ClassId::LABELABBR);
    if (!label) {
        return "";
    }
    Object *obj = label->FindDescendantByType(ClassId::TEXT);
    if (!obj) {
        return "";
    }
    Text *text = (Text *)obj;
    std::string name = UTF32to8(text->GetText());
    return name;
}

//////////////////////////////
//
// HumdrumInput::processStaffDecoration -- Currently only one level
//    of bracing is allowed.  Probably allow remapping of staff order
//    with the system decoration, and possible merge two kern spines
//    onto a single staff (such as two similar instruments sharing
//    a common staff).
//

bool HumdrumInput::processStaffDecoration(const std::string &decoration)
{
    if (decoration.empty()) {
        return false;
    }
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    std::vector<int> tracklist;
    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        int track = staffstarts[i]->getTrack();
        tracklist.push_back(track);
    }

    bool validQ = true;

    // If decoration prefixes number with "s", then match to a kern
    // start which contains *staff# before any data content.  If the
    // number is not prefixed by "s", then assume that it is a kern
    // spine enumeration.  Staff enumerations can be utilized if a smaller
    // group of parts are extracted, but kern enumerations can become
    // invalid if extracting sub-scores.  In both cases the enumeration
    // goes from the top of the staff to the bottom, which means from right
    // to left on the Humdrum line.  If the order of the staff or kern
    // enumeration in the decoration is not monotonic covering every staff
    // in the score, then the results may have problems.

    std::map<std::string, std::vector<int>> classToStaffMapping;
    std::map<int, std::vector<int>> groupToStaffMapping;
    std::map<int, std::vector<int>> partToStaffMapping;
    std::map<int, int> trackToSpineMapping;
    std::map<int, int> staffToSpineMapping;
    std::map<int, int> staffToGroupMapping;
    std::map<int, std::string> staffToClassMapping;
    std::map<int, int> spineToGroupMapping;
    std::map<int, int> staffToPartMapping;
    std::map<int, int> spineToPartMapping;

    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        int staff = getStaffNumberLabel(staffstarts[i]);
        int group = getGroupNumberLabel(staffstarts[i]);
        int part = getPartNumberLabel(staffstarts[i]);
        int track = staffstarts[i]->getTrack();
        std::string instrumentClass = getInstrumentClass(staffstarts[i]);

        trackToSpineMapping[track] = i;

        if (!instrumentClass.empty()) {
            classToStaffMapping[instrumentClass].push_back(staff);
            staffToClassMapping[i] = instrumentClass;
        }
        if (group > 0) {
            groupToStaffMapping[group].push_back(staff);
            spineToGroupMapping[i] = group;
        }
        if (part > 0) {
            partToStaffMapping[part].push_back(staff);
            spineToPartMapping[i] = part;
        }
        if (staff > 0) {
            staffToSpineMapping[staff] = i;
        }
        if ((group > 0) && (staff > 0)) {
            staffToGroupMapping[staff] = group;
        }
        if ((part > 0) && (staff > 0)) {
            staffToPartMapping[staff] = part;
        }
        if ((!instrumentClass.empty()) && (staff > 0)) {
            staffToClassMapping[staff] = instrumentClass;
        }
    }

    // Expand groupings into staves.  The d variable contains the expansions
    // and the decoration variable contains the original decoration string.
    std::string d = decoration;

    // Instrument class expansion to staff numbers:
    hum::HumRegex hre;
    if (!classToStaffMapping.empty()) {
        for (auto it = classToStaffMapping.begin(); it != classToStaffMapping.end(); it++) {
            std::string pattern = it->first;
            std::string replacement = "";
            for (int i = 0; i < (int)it->second.size(); ++i) {
                replacement += "s" + to_string(it->second[i]);
            }
            hre.replaceDestructive(d, replacement, pattern, "g");
        }
    }

    // The group-to-staff substitution is limited
    // to single-digit group numbers for now.
    if (!groupToStaffMapping.empty()) {
        // substitute spine groupings with staff numbers.
        // example:   {(g1}} will be expanded to {(s1,s2,s3)} if
        // group1 is given to staff1, staff2, and staff3.
        std::string gstring;
        std::string sstring;
        for (auto const &it : groupToStaffMapping) {
            gstring = "g" + to_string(it.first);
            sstring = "";
            for (int i = 0; i < (int)it.second.size(); ++i) {
                sstring += "s" + to_string(it.second.at(i));
                if (i < (int)it.second.size() - 1) {
                    sstring += ",";
                }
            }
            hre.replaceDestructive(d, sstring, gstring);
        }
    }

    // Expand parts to staves.
    if (!partToStaffMapping.empty()) {
        // substitute spine parts with staff numbers.
        // example:   {(p1}} will be expanded to {(s1,s2)} if
        // part1 is given to staff1 and staff2.
        std::string pstring;
        std::string sstring;
        for (auto const &it : partToStaffMapping) {
            pstring = "p" + to_string(it.first);
            sstring = "";
            for (int i = 0; i < (int)it.second.size(); ++i) {
                sstring += "s" + to_string(it.second.at(i));
                if (i < (int)it.second.size() - 1) {
                    sstring += ",";
                }
            }
            hre.replaceDestructive(d, sstring, pstring);
        }
    }

    // remove unexpanded groups and parts
    hre.replaceDestructive(d, "", "p\\d+", "g");
    hre.replaceDestructive(d, "", "g\\d+", "g");

    // Remove any invalid characters:
    hre.replaceDestructive(d, "", "[^0-9s()<>{}*\\][]", "g");

    // Expand * to mean all staves present in score.
    bool hasstar = false;
    if (hre.search(d, "\\*")) {
        std::string tstring;
        for (int i = 0; i < (int)tracklist.size(); ++i) {
            tstring += "t" + to_string(tracklist[i]);
        }
        hre.replaceDestructive(d, tstring, "\\*");
        hasstar = true;
    }
    hre.replaceDestructive(d, "", "[*]", "g");

    if (d.empty()) {
        return false;
    }
    if ((0)) {
        cerr << "INPUT DECORATION: " << decoration << endl;
        cerr << "     PROCESSED:   " << d << endl;
    }

    // Remove any staff numbers that are no longer present (or invalid):
    std::vector<int> deconums = getStaffNumbers(d);
    for (int i = 0; i < (int)deconums.size(); ++i) {
        auto it = staffToSpineMapping.find(deconums.at(i));
        if (it != staffToSpineMapping.end()) {
            continue;
        }
        // The staff number in the decoration string is not present
        // in the list so remove it.
        std::string target = "s";
        target += to_string(deconums.at(i));
        target += "(?!\\d)";
        hre.replaceDestructive(d, "", target);
    }
    // Remove any empty groups:
    hre.replaceDestructive(d, "", "\\(\\)", "g");
    hre.replaceDestructive(d, "", "\\{\\}", "g");
    hre.replaceDestructive(d, "", "\\[\\]", "g");
    hre.replaceDestructive(d, "", "<>", "g");
    // Do it again to be safe (for one recursion):
    hre.replaceDestructive(d, "", "\\(\\)", "g");
    hre.replaceDestructive(d, "", "\\{\\}", "g");
    hre.replaceDestructive(d, "", "<>", "g");

    int scount = 0;
    for (int i = 0; i < (int)d.size(); ++i) {
        if (d[i] == 's') {
            scount++;
        }
        else if (d[i] == 't') {
            scount++;
        }
    }
    if (scount == 0) {
        return false;
    }
    if (scount == 1) {
        // Remove decoration when a single staff on system.
        hre.replaceDestructive(d, "", "[^ts\\d]", "g");
    }

    // Now pair (), <> {}, and [] parentheses in the d string.
    std::vector<pair<int, char>> stack;
    pair<int, char> item;
    std::vector<int> pairing(d.size(), -1);
    for (int i = 0; i < (int)d.size(); ++i) {
        if (d[i] == '(') {
            item.first = i;
            item.second = d[i];
            stack.push_back(item);
        }
        else if (d[i] == '{') {
            item.first = i;
            item.second = d[i];
            stack.push_back(item);
        }
        else if (d[i] == '[') {
            item.first = i;
            item.second = d[i];
            stack.push_back(item);
        }
        else if (d[i] == '<') {
            item.first = i;
            item.second = d[i];
            stack.push_back(item);
        }
        else if (d[i] == ')') {
            if (stack.empty()) {
                validQ = false;
                break;
            }
            if (stack.back().second != '(') {
                validQ = false;
                break;
            }
            pairing.at(stack.back().first) = i;
            pairing.at(i) = stack.back().first;
            stack.resize((int)stack.size() - 1);
        }
        else if (d[i] == '}') {
            if (stack.empty()) {
                validQ = false;
                break;
            }
            if (stack.back().second != '{') {
                validQ = false;
                break;
            }
            pairing.at(stack.back().first) = i;
            pairing.at(i) = stack.back().first;
            stack.resize((int)stack.size() - 1);
        }
        else if (d[i] == ']') {
            if (stack.empty()) {
                validQ = false;
                break;
            }
            if (stack.back().second != '[') {
                validQ = false;
                break;
            }
            pairing.at(stack.back().first) = i;
            pairing.at(i) = stack.back().first;
            stack.resize((int)stack.size() - 1);
        }
        else if (d[i] == '>') {
            if (stack.empty()) {
                validQ = false;
                break;
            }
            if (stack.back().second != '<') {
                validQ = false;
                break;
            }
            pairing.at(stack.back().first) = i;
            pairing.at(i) = stack.back().first;
            stack.resize((int)stack.size() - 1);
        }
    }
    if (!stack.empty()) {
        // open/close not paired correctly
        validQ = false;
    }

    if ((0)) {
        // print analysis:
        for (int i = 0; i < (int)d.size(); ++i) {
            cerr << "D[" << i << "] =\t" << d[i] << " pairing: " << pairing[i] << endl;
        }
    }

    if (!validQ) {
        return false;
    }

    if (pairing.empty()) {
        return false;
    }

    bool skipfirst = false;
    bool skipsecond = false;
    StaffGrp *root = NULL;
    if (pairing.back() != 0) {
        // There is no barline across the staves in this case.
        root = new StaffGrp();
        root->SetBarThru(BOOLEAN_false);
        m_doc->GetCurrentScoreDef()->AddChild(root);
    }
    else if (d[0] == '(') {
        // The outer group is not bracketed, but bar goes all of
        // the way through system.
        root = new StaffGrp();
        root->SetBarThru(BOOLEAN_true);
        m_doc->GetCurrentScoreDef()->AddChild(root);
    }
    else if (pairing.back() == 0) {
        skipfirst = true;
        // some sort of grouping for the entire system
        root = new StaffGrp();
        if ((pairing.size() > 1) && (d[1] == '(')) {
            skipsecond = true;
            root->SetBarThru(BOOLEAN_true);
        }
        if (d[0] == '{') {
            setGroupSymbol(root, staffGroupingSym_SYMBOL_brace);
        }
        else if (d[0] == '[') {
            setGroupSymbol(root, staffGroupingSym_SYMBOL_bracket);
        }
        m_doc->GetCurrentScoreDef()->AddChild(root);
    }

    std::vector<int> spine; // kernstart index
    std::vector<bool> barstart; // true if bar group starts at staff.
    std::vector<bool> barend; // true if bar group stops at staff.

    std::vector<std::vector<int>> bargroups;
    std::vector<std::string> groupstyle;

    groupstyle.resize(1);
    groupstyle.back() = " ";
    bargroups.resize(1);

    bool staffQ = false;
    bool trackQ = false;
    int value = 0;
    bool grouper = false;
    int glevel = 0;

    int start = 0;
    int ending = (int)d.size();
    if (skipfirst) {
        start = 1;
        ending--;
    }
    if (skipsecond) {
        start = 2;
        ending--;
    }

    for (int i = start; i < ending; ++i) {
        if (d[i] == '[') {
            if (!grouper) {
                if (bargroups.back().empty()) {
                    groupstyle.back() = "[";
                }
                else {
                    groupstyle.push_back("[");
                    bargroups.resize(bargroups.size() + 1);
                }
            }
            groupstyle.back() = "[";
            if (i < (int)d.size() - 1) {
                if (d[i + 1] == '(') {
                    groupstyle.back() += "(";
                    i++;
                }
            }
            grouper = true;
            glevel++;
        }

        else if (d[i] == '{') {
            if (!grouper) {
                if (bargroups.back().empty()) {
                    groupstyle.back() = "{";
                }
                else {
                    groupstyle.push_back("{");
                    bargroups.resize(bargroups.size() + 1);
                }
            }
            groupstyle.back() = "{";
            if (i < (int)d.size() - 1) {
                if (d[i + 1] == '(') {
                    groupstyle.back() += "(";
                    i++;
                }
            }
            grouper = true;
            glevel++;
        }

        else if (d[i] == '<') {
            if (!grouper) {
                if (bargroups.back().empty()) {
                    groupstyle.back() = "<";
                }
                else {
                    groupstyle.push_back("<");
                    bargroups.resize(bargroups.size() + 1);
                }
            }
            groupstyle.back() = "<";
            if (i < (int)d.size() - 1) {
                if (d[i + 1] == '(') {
                    groupstyle.back() += "(";
                    i++;
                }
            }
            grouper = true;
            glevel++;
        }

        else if (d[i] == '}') {
            groupstyle.push_back(" ");
            bargroups.resize(bargroups.size() + 1);
            glevel--;
            if (glevel == 0) {
                grouper = false;
            }
        }

        else if (d[i] == ']') {
            groupstyle.push_back(" ");
            bargroups.resize(bargroups.size() + 1);
            glevel--;
            if (glevel == 0) {
                grouper = false;
            }
        }

        else if (d[i] == '>') {
            groupstyle.push_back(" ");
            bargroups.resize(bargroups.size() + 1);
            glevel--;
            if (glevel == 0) {
                grouper = false;
            }
        }

        else if (d[i] == 's') {
            staffQ = true;
            trackQ = false;
        }
        else if (d[i] == 't') {
            staffQ = false;
            trackQ = true;
        }
        else if (d[i] == '(') {
            groupstyle.back() = "(";
            grouper = true;
        }
        else if (d[i] == ')') {
            if ((groupstyle.back().size() > 1) && (groupstyle.back().at(1) == '(')) {
                // ignore since it does not indicate a group.
            }
            else {
                // End of a bar group without extra decoration:
                bargroups.resize(bargroups.size() + 1);
                groupstyle.push_back(" ");
            }
            glevel--;
            if (glevel == 0) {
                grouper = false;
            }
        }
        else if (std::isdigit(d[i])) {
            if (value < 0) {
                value = 0;
            }
            value = value * 10 + (d[i] - '0');
            if ((i == (int)d.size() - 1) || !std::isdigit(d[i + 1])) {
                if (staffQ) {
                    auto it = staffToSpineMapping.find(value);
                    if (it != staffToSpineMapping.end()) {
                        value = (*it).second;
                    }
                    else {
                        value = -1;
                    }
                }
                else if (trackQ) {
                    value = trackToSpineMapping[value];
                }
                staffQ = false;
                trackQ = false;
                if (value < 0) {
                    // Spine does not exist in score, so skip
                    value = 0;
                    continue;
                }
                else {
                    bargroups.back().push_back(value);
                }
                value = 0;
            }
        }
    }

    if ((!bargroups.empty()) && bargroups.back().empty()) {
        bargroups.resize((int)bargroups.size() - 1);
    }

    if ((0)) {
        cerr << "BAR GROUPS" << endl;
        for (int i = 0; i < (int)bargroups.size(); ++i) {
            cerr << "\tgroup_style=" << groupstyle[i] << "\tgroup = " << i << ":\t";
            for (int j = 0; j < (int)bargroups[i].size(); j++) {
                cerr << " " << bargroups[i][j];
            }
            cerr << endl;
        }
    }

    // Pull out all non-zero staff groups:
    std::vector<std::vector<int>> newgroups;
    std::vector<std::string> newstyles;
    for (int i = 0; i < (int)bargroups.size(); ++i) {
        if (bargroups[i].empty()) {
            continue;
        }
        newgroups.push_back(bargroups[i]);
        newstyles.push_back(groupstyle[i]);
    }

    // Check to see that all staffstarts are represented in system decoration;
    // otherwise, declare that it is invalid and print a simple decoration.
    std::vector<int> found(staffstarts.size(), 0);
    if (hasstar) {
        std::fill(found.begin(), found.end(), 1);
    }
    else {

        for (int i = 0; i < (int)newgroups.size(); ++i) {
            for (int j = 0; j < (int)newgroups[i].size(); ++j) {
                found.at(newgroups[i][j])++;
            }
        }

        for (int i = 0; i < (int)found.size(); ++i) {
            if (found[i] != 1) {
                cerr << "I:" << i << "\t=\t" << found[i] << endl;
                validQ = false;
                break;
            }
        }
    }

    if (!validQ) {
        cerr << "DECORATION IS INVALID " << decoration << endl;
        if (d != decoration) {
            cerr << "\tSTAFF VERSION: " << d << endl;
        }
        StaffGrp *sg = new StaffGrp();
        setGroupSymbol(sg, staffGroupingSym_SYMBOL_bracket);
        if (root) {
            root->AddChild(sg);
        }
        else {
            m_doc->GetCurrentScoreDef()->AddChild(sg);
        }
        for (int i = 0; i < (int)m_staffdef.size(); ++i) {
            sg->AddChild(m_staffdef[i]);
        }
        // returning true since the staff groups are
        // dealt with at this point.
        return true;
    }

    // Build system groups based on system decoration instructions
    if (newgroups.size() == 1) {
        // only one group, but already added
        StaffGrp *sg = NULL;
        if (root) {
            sg = root;
        }
        else {
            sg = new StaffGrp();
            if (root) {
                root->AddChild(sg);
            }
            else {
                m_doc->GetCurrentScoreDef()->AddChild(sg);
            }
        }

        std::string groupName = "";
        hum::HTp groupNameTok = NULL;
        std::string groupAbbr = "";
        hum::HTp groupAbbrTok = NULL;
        int mygroup = -1;
        if (!newgroups[0].empty()) {
            mygroup = spineToGroupMapping[newgroups[0][0]];
            if (mygroup > 0) {
                groupName = m_group_name[mygroup];
                groupNameTok = m_group_name_tok[mygroup];
                groupAbbr = m_group_abbr[mygroup];
                groupAbbrTok = m_group_abbr_tok[mygroup];
            }
        }
        if ((!groupAbbr.empty()) && (groupAbbrTok != NULL)) {
            setInstrumentAbbreviation(sg, groupAbbr, groupAbbrTok);
        }
        if ((!groupName.empty()) && (groupNameTok != NULL)) {
            setInstrumentName(sg, groupName, groupNameTok);
        }

        if ((!newstyles.at(0).empty()) && (newstyles.at(0).at(0) == '[')) {
            if (newgroups.at(0).size() > 1) {
                setGroupSymbol(sg, staffGroupingSym_SYMBOL_bracket);
            }
            if (newstyles.at(0).find('(') != std::string::npos) {
                sg->SetBarThru(BOOLEAN_true);
            }
            else {
                sg->SetBarThru(BOOLEAN_false);
            }
        }

        else if ((!newstyles.at(0).empty()) && (newstyles.at(0).at(0) == '{')) {
            if (newgroups.at(0).size() > 1) {
                setGroupSymbol(sg, staffGroupingSym_SYMBOL_brace);
            }
            if (newstyles.at(0).find('(') != std::string::npos) {
                sg->SetBarThru(BOOLEAN_true);
            }
            else {
                sg->SetBarThru(BOOLEAN_false);
            }
        }

        else if ((!newstyles.at(0).empty()) && (newstyles.at(0).at(0) == '<')) {
            if (newgroups.at(0).size() > 1) {
                // setGroupSymbol(sg, staffGroupingSym_SYMBOL_brace);
            }
            if (newstyles.at(0).find('(') != std::string::npos) {
                sg->SetBarThru(BOOLEAN_true);
            }
            else {
                sg->SetBarThru(BOOLEAN_false);
            }
        }

        else if ((!newstyles.at(0).empty()) && (newstyles.at(0).at(0) == '(')) {
            sg->SetBarThru(BOOLEAN_true);
        }

        for (int i = 0; i < (int)newgroups[0].size(); ++i) {
            sg->AddChild(m_staffdef[newgroups[0][i]]);
        }
    }
    else {
        // An all-staff group should have already been created.
        StaffGrp *root_sg = NULL;
        if (root) {
            root_sg = root;
        }
        else {
            root_sg = new StaffGrp();
            m_doc->GetCurrentScoreDef()->AddChild(root_sg);
            root_sg->SetBarThru(BOOLEAN_false);
        }
        for (int i = 0; i < (int)newgroups.size(); ++i) {
            // create staffGrp and then insert staffDefs for group
            StaffGrp *sg = new StaffGrp();
            root_sg->AddChild(sg);
            if (groupstyle[i].find("(") != std::string::npos) {
                sg->SetBarThru(BOOLEAN_true);
            }
            else {
                sg->SetBarThru(BOOLEAN_false);
            }

            std::string groupName = "";
            hum::HTp groupNameTok = NULL;
            std::string groupAbbr = "";
            hum::HTp groupAbbrTok = NULL;
            int mygroup = -1;
            if (!newgroups[i].empty()) {
                mygroup = spineToGroupMapping[newgroups[i][0]];
                if (mygroup > 0) {
                    groupName = m_group_name[mygroup];
                    groupNameTok = m_group_name_tok[mygroup];
                    groupAbbr = m_group_abbr[mygroup];
                    groupAbbrTok = m_group_abbr_tok[mygroup];
                }
            }
            if ((!groupAbbr.empty()) && (groupAbbrTok != NULL)) {
                setInstrumentAbbreviation(sg, groupAbbr, groupAbbrTok);
            }
            if ((!groupName.empty()) && (groupNameTok != NULL)) {
                setInstrumentName(sg, groupName, groupNameTok);
            }

            if (newstyles.at(i).at(0) == '[') {
                setGroupSymbol(sg, staffGroupingSym_SYMBOL_bracket);
            }
            else if (newstyles.at(i).at(0) == '{') {
                setGroupSymbol(sg, staffGroupingSym_SYMBOL_brace);
            }
            else if (newstyles.at(i).at(0) == '<') {
                // setGroupSymbol(sg, staffGroupingSym_SYMBOL_brace);
            }
            for (int j = 0; j < (int)newgroups[i].size(); ++j) {
                sg->AddChild(m_staffdef[newgroups[i][j]]);
            }
            // add label and label abbreviation to group
        }
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::setGroupSymbol -- Add a StaffGrp@symbol as well as
//   promote it to a child element (that verovio uses to actually display
//   the symbol when rendering to SVG).

void HumdrumInput::setGroupSymbol(StaffGrp *sg, staffGroupingSym_SYMBOL symbol)
{
    // Do not set the symbol on the StaffGrp, since it will be taken from
    // the GrpSym child element and inserted into StaffGrp when writing MEI.
    // sg->SetSymbol(symbol);

    // Then add as a child element of <StaffGrp> which verovio uses to
    // display group symbol in SVG export:
    GrpSym *gs = new GrpSym();
    gs->IsAttribute(true); // Copy of an attribute in the parent element.
    gs->SetSymbol(symbol);
    sg->AddChild(gs);
}

//////////////////////////////
//
// HumdrumInput::getInstrumentClass -- Get the string after *IC at the start
//   of a spine before any data or spine splits.
//

std::string HumdrumInput::getInstrumentClass(hum::HTp start)
{
    hum::HTp current = start;
    std::string output;
    while (current) {
        if (current->isData()) {
            break;
        }
        if (!current->isInterpretation()) {
            current = current->getNextToken();
            continue;
        }
        if (current->compare(0, 3, "*IC") == 0) {
            output = current->substr(3);
            break;
        }
        current = current->getNextToken();
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::getStaffNumbers -- Extract numbers from list.
//

std::vector<int> HumdrumInput::getStaffNumbers(std::string &deco)
{
    std::vector<int> output;
    for (int i = 0; i < (int)deco.size(); ++i) {
        if (isdigit(deco[i])) {
            int value = 0;
            while ((i < (int)deco.size()) && (isdigit(deco[i]))) {
                value = value * 10 + (deco[i] - '0');
                i++;
            }
            output.push_back(value);
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::prepareHeaderFooter --
//
//

void HumdrumInput::prepareHeaderFooter()
{
    hum::HumdrumFile &infile = m_infiles[0];
    std::vector<std::pair<std::string, std::string>> biblist;

    hum::HumRegex hre;
    std::vector<hum::HumdrumLine *> records = infile.getReferenceRecords();
    biblist.reserve(records.size());
    std::map<std::string, std::string> refmap;
    for (int i = 0; i < (int)records.size(); ++i) {
        std::string key = records[i]->getReferenceKey();
        std::string value = records[i]->getReferenceValue();
        refmap[key] = value;
        biblist.emplace_back(std::make_pair(key, value));
    }

    prepareHeader(biblist, refmap);
    prepareFooter(biblist, refmap);
}

//////////////////////////////
//
// HumdrumInput::processTemplateOperator --
//      Operators:
//           n = full name
//           i = initials for given names and full last name
//           l = last name
//           f = first name
//           y = year
//

std::string HumdrumInput::processTemplateOperator(const std::string &value, const std::string &op)
{
    std::string input = value;
    std::string output;
    hum::HumRegex hre;

    if (op.find("U") != std::string::npos) {
        // Force all upper case (cannot handle HTML accents, so should be unescaped first)
        std::locale loc;
        for (int i = 0; i < (int)input.size(); ++i) {
            input[i] = std::toupper(input[i], loc);
        }
    }

    if (op.find("n") != std::string::npos) {
        // full name: reverse strings separated by comma if there is one.
        auto pos = input.find(",");
        if (pos == std::string::npos) {
            output = input;
        }
        else {
            std::string lastname = input.substr(0, pos);
            std::string firstname = input.substr(pos + 1);
            output = firstname + " " + lastname;
        }
    }
    else if (op.find("l") != std::string::npos) {
        // last name: text before comma if there is one.
        auto pos = input.find(",");
        if (pos == std::string::npos) {
            output = input;
        }
        else {
            std::string lastname = input.substr(0, pos);
            output = lastname;
        }
    }
    else if (op.find("f") != std::string::npos) {
        // first name: text after comma if there is one.
        auto pos = input.find(",");
        if (pos == std::string::npos) {
            output = input;
        }
        else {
            std::string firstname = input.substr(pos + 1);
            output = firstname;
        }
    }
    else if (op.find("i") != std::string::npos) {
        // initialed name: first name converted to initials
        auto pos = input.find(",");
        if (pos == std::string::npos) {
            output = input;
        }
        else {
            std::string lastname = input.substr(0, pos);
            std::string firstname = input.substr(pos + 1);
            std::vector<std::string> pieces;
            hre.split(pieces, firstname, " ");
            firstname.clear();
            for (int i = 0; i < (int)pieces.size(); ++i) {
                if (pieces[i].empty()) {
                    continue;
                }
                if (std::isupper(pieces[i][0])) {
                    pieces[i] = pieces[i][0];
                    pieces[i] += ".";
                    if (firstname.size() > 1) {
                        if (firstname[firstname.size() - 2] == '.') {
                            firstname.pop_back();
                            firstname += pieces[i] + " ";
                        }
                        else {
                            firstname += pieces[i] + " ";
                        }
                    }
                    else {
                        firstname += pieces[i] + " ";
                    }
                }
                else {
                    firstname += pieces[i] + " ";
                }
            }
            output = firstname + lastname;
        }
    }
    else if (op.find("y") != std::string::npos) {
        // Show only years for birth/death, and shorten if in same century.
        // Still need to include circa, flourish, and birth/death only.
        std::string cdates = input;
        std::string outputdate;
        std::string birth;
        std::string death;
        auto pos = cdates.find("-");
        if (pos != std::string::npos) {
            birth = cdates.substr(0, pos);
            death = cdates.substr(pos + 1);
            int birthyear = 0;
            int deathyear = 0;
            if (hre.search(birth, "(\\d{4})")) {
                birthyear = hre.getMatchInt(1);
            }
            if (hre.search(death, "(\\d{4})")) {
                deathyear = hre.getMatchInt(1);
            }
            if ((deathyear > 0) && (birthyear > 0)) {
                outputdate = to_string(birthyear);
                outputdate += "&#8211;";
                if ((deathyear / 100) == (birthyear / 100)) {
                    outputdate += to_string(deathyear % 100);
                }
                else {
                    outputdate += to_string(deathyear);
                }
            }
        }
        else {
            if (hre.search(cdates, "(\\d{4})")) {
                output = hre.getMatch(1);
            }
        }
        output = outputdate;
    }
    else if (op.find("Y") != std::string::npos) {
        // Show only years for birth/death, but do not shorten if in same century.
        // Still need to include circa, flourish, and birth/death only.
        std::string cdates = input;
        std::string outputdate;
        std::string birth;
        std::string death;
        auto pos = cdates.find("-");
        if (pos != std::string::npos) {
            birth = cdates.substr(0, pos);
            death = cdates.substr(pos + 1);
            int birthyear = 0;
            int deathyear = 0;
            if (hre.search(birth, "(\\d{4})")) {
                birthyear = hre.getMatchInt(1);
            }
            if (hre.search(death, "(\\d{4})")) {
                deathyear = hre.getMatchInt(1);
            }
            if ((deathyear > 0) && (birthyear > 0)) {
                outputdate = to_string(birthyear);
                outputdate += "&#8211;";
                outputdate += to_string(deathyear);
            }
        }
        else {
            if (hre.search(cdates, "(\\d{4})")) {
                output = hre.getMatch(1);
            }
        }
        output = outputdate;
    }
    else {
        // unknown operator, so just echo input
        output = input;
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::processReferenceTemplate --
//

std::string HumdrumInput::processReferenceTemplate(const std::string &input,
    std::vector<std::pair<std::string, std::string>> &biblist, std::map<std::string, std::string> &refmap)
{
    std::string text = input;
    hum::HumRegex hre;
    std::string prefix;
    std::string suffix;
    std::string match;
    std::string key;
    std::string op;
    while (hre.search(text, "@\\{(.*?)\\}")) {
        match = hre.getMatch(1);
        prefix = hre.getPrefix();
        suffix = hre.getSuffix();
        auto pos = match.find(":");
        if (pos != std::string::npos) {
            key = match.substr(0, pos);
            op = match.substr(pos + 1);
            match = processTemplateOperator(refmap[key], op);
        }
        else {
            match = refmap[match];
        }
        text = prefix;
        text += match;
        text += suffix;
    }
    return unescapeHtmlEntities(text);
}

//////////////////////////////
//
// HumdrumInput::prepareFooter --
//

bool HumdrumInput::prepareFooter(
    std::vector<std::pair<std::string, std::string>> &biblist, std::map<std::string, std::string> &refmap)
{

    std::string footleft;
    std::string footcenter;
    std::string footright;
    std::vector<std::string> pieces;
    std::string tstring;
    hum::HumRegex hre;

    //   <rend halign="left" valign="bottom">
    //      <rend fontsize="small">Footer L1</rend><lb/>
    //      <rend fontsize="small">Footer L2</rend><lb/>
    //      <rend fontsize="small">Footer L3</rend>
    //   </rend>

    auto itL = refmap.find("footer-left");
    if (itL != refmap.end()) {
        hre.split(pieces, itL->second, "\\\\n");
        footleft += "<rend halign=\"left\" valign=\"bottom\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            footleft += "<rend fontsize=\"small\">";
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                footleft += "&#160;";
            }
            else {
                footleft += tstring;
            }
            footleft += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                footleft += "<lb/>\n";
            }
        }
        footleft += "</rend>\n";
    }

    auto itC = refmap.find("footer-center");
    if (itC != refmap.end()) {
        hre.split(pieces, itC->second, "\\\\n");
        footcenter += "<rend halign=\"center\" valign=\"bottom\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            footcenter += "<rend fontsize=\"small\">";
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                footcenter += "&#160;";
            }
            else {
                footcenter += tstring;
            }
            footcenter += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                footcenter += "<lb/>\n";
            }
        }
        footcenter += "</rend>\n";
    }

    auto itR = refmap.find("footer-right");
    if (itR != refmap.end()) {
        hre.split(pieces, itR->second, "\\\\n");
        footright += "<rend halign=\"right\" valign=\"bottom\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            footright += "<rend fontsize=\"small\">";
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                footright += "&#160;";
            }
            else {
                footright += tstring;
            }
            footright += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                footright += "<lb/>\n";
            }
        }
        footright += "</rend>\n";
    }

    std::string footer;
    footer += footleft;
    footer += footcenter;
    footer += footright;

    if (footer.empty()) {
        return false;
    }

    hre.replaceDestructive(footer, "</rend>", "</i>", "g");
    hre.replaceDestructive(footer, "<rend fontstyle=\"italic\">", "<i>", "g");
    hre.replaceDestructive(footer, "<rend><num label=\"page\">#</num></rend>", "%P", "g");

    std::string meifile = "<mei xmlns=\"http://www.music-encoding.org/ns/mei\" meiversion=\"4.0.0\">\n";
    meifile += "<meiHead></meiHead>";
    meifile += "<music><body><mdiv><score><scoreDef>\n";
    meifile += "<pgFoot>\n";
    meifile += footer;
    meifile += "</pgFoot>\n";
    // Always putting footer on all pages for now:
    meifile += "<pgFoot2>\n";
    meifile += footer;
    meifile += "</pgFoot2>\n";
    meifile += "</scoreDef></score></mdiv></body></music></mei>\n";

    Doc tempdoc;
    MEIInput input(&tempdoc);
    if (!input.Import(meifile)) {
        LogError("Error importing data");
        return false;
    }

    // MEIOutput meioutput(&tempdoc);
    // meioutput.SetScoreBasedMEI(true);
    // std::string meicontent = meioutput.GetOutput();
    // std::cout << "MEI CONTENT " << meicontent << std::endl;

    Object *pgfoot = tempdoc.GetCurrentScoreDef()->FindDescendantByType(ClassId::PGFOOT);
    if (pgfoot == NULL) {
        return false;
    }

    int index = pgfoot->GetIdx();
    if (index < 0) {
        return false;
    }
    Object *detached = pgfoot->GetParent()->DetachChild(index);
    if (detached != pgfoot) {
        std::cerr << "Detached element is not the pgHead" << std::endl;
        if (detached) {
            delete detached;
        }
        return false;
    }

    m_doc->GetCurrentScoreDef()->AddChild(pgfoot);

    Object *pgfoot2 = tempdoc.GetCurrentScoreDef()->FindDescendantByType(ClassId::PGFOOT2);
    if (pgfoot2 == NULL) {
        return true;
    }

    index = pgfoot2->GetIdx();
    if (index < 0) {
        return true;
    }
    detached = pgfoot2->GetParent()->DetachChild(index);
    if (detached != pgfoot2) {
        std::cerr << "Detached element is not a pgFoot2 element" << std::endl;
        if (detached) {
            delete detached;
        }
        return true;
    }

    m_doc->GetCurrentScoreDef()->AddChild(pgfoot2);

    return true;
}

//////////////////////////////
//
// HumdrumInput::prepareHeader --
//
// <pgHead>
//     <rend halign="center" valign="middle">
//         <rend fontsize="x-large">Non è questa la mano (<rend fontstyle="italic">Rime</rend> 47)</rend>
//         <lb/>
//         <rend fontsize="small"><rend fontstyle="italic">Il primo libro de madrigali a quattro voci. </rend>(Venice:
//         Gardano, 1579)</rend>
//     </rend>
//     <rend halign="right" valign="bottom">Benedetto Pallavicino</rend>
// </pgHead>
//

bool HumdrumInput::prepareHeader(
    std::vector<std::pair<std::string, std::string>> &biblist, std::map<std::string, std::string> &refmap)
{
    std::string headleft;
    std::string headcenter;
    std::string headright;
    std::string tstring;
    hum::HumRegex hre;
    std::vector<std::string> pieces;

    auto ithc = refmap.find("header-center");
    if (ithc != refmap.end()) {
        hre.split(pieces, ithc->second, "\\\\n");
        headcenter = "<rend halign=\"center\" valign=\"middle\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            if (i == 0) {
                headcenter += "<rend fontsize=\"large\">";
            }
            else {
                headcenter += "<rend fontsize=\"normal\">";
            }
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                headcenter += "&#160;";
            }
            else {
                headcenter += tstring;
            }
            headcenter += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                headcenter += "<lb/>\n";
            }
        }
        headcenter += "</rend>\n";
    }
    else {
        headcenter = automaticHeaderCenter(biblist, refmap);
    }

    int linecount = 0;
    auto ithr = refmap.find("header-right");
    if (ithr != refmap.end()) {
        hre.split(pieces, ithr->second, "\\\\n");
        headright = "<rend halign=\"right\" valign=\"bottom\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            headright += "<rend fontsize=\"small\">";
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                headright += "&#160;";
            }
            else {
                headright += tstring;
            }
            headright += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                headright += "<lb/>\n";
            }
        }
        headright += "</rend>\n";
    }
    else {
        headright = automaticHeaderRight(biblist, refmap, linecount);
    }

    auto ithl = refmap.find("header-left");
    if (ithl != refmap.end()) {
        hre.split(pieces, ithl->second, "\\\\n");
        headleft = "<rend halign=\"left\" valign=\"bottom\">\n";
        for (int i = 0; i < (int)pieces.size(); ++i) {
            headleft += "<rend fontsize=\"small\">";
            tstring = processReferenceTemplate(pieces[i], biblist, refmap);
            if (pieces[i].empty()) {
                headleft += "&#160;";
            }
            else {
                headleft += tstring;
            }
            headleft += "</rend>\n";
            if (i < (int)pieces.size() - 1) {
                headleft += "<lb/>\n";
            }
        }
        headleft += "</rend>\n";
    }
    else {
        headleft = automaticHeaderLeft(biblist, refmap, linecount);
    }

    std::string head = headcenter + headleft + headright;

    if (head.empty()) {
        return false;
    }

    hre.replaceDestructive(head, "</rend>", "</i>", "g");
    hre.replaceDestructive(head, "<rend fontstyle=\"italic\">", "<i>", "g");
    hre.replaceDestructive(head, "<rend><num label=\"page\">#</num></rend>", "%P", "g");

    std::string meifile = "<mei xmlns=\"http://www.music-encoding.org/ns/mei\" meiversion=\"4.0.0\">\n";
    meifile += "<meiHead></meiHead>";
    meifile += "<music><body><mdiv><score><scoreDef><pgHead>\n";
    meifile += head;
    meifile += "</pgHead></scoreDef></score></mdiv></body></music></mei>\n";

    Doc tempdoc;
    MEIInput input(&tempdoc);
    if (!input.Import(meifile)) {
        LogError("Error importing data");
        return false;
    }

    // MEIOutput meioutput(&tempdoc);
    // meioutput.SetScoreBasedMEI(true);
    // std::string meicontent = meioutput.GetOutput();
    // std::cout << "MEI CONTENT " << meicontent << std::endl;

    Object *pghead = tempdoc.GetCurrentScoreDef()->FindDescendantByType(ClassId::PGHEAD);
    if (pghead == NULL) {
        return false;
    }

    int index = pghead->GetIdx();
    if (index < 0) {
        return false;
    }
    Object *detached = pghead->GetParent()->DetachChild(index);
    if (detached != pghead) {
        std::cerr << "Detached element is not the pgHead" << std::endl;
        if (detached) {
            delete detached;
        }
        return false;
    }

    m_doc->GetCurrentScoreDef()->AddChild(pghead);

    return true;
}

//////////////////////////////
//
// HumdrumInput::automaticHeaderLeft -- If there is no reference record "header-left",
//     Then any "LYR" (lyricist) will be displayed, and if LDT (lyric composition date)
//     is found, then that will be added in parenthese after the lyricist's name.
//     The linecount variable is used to align the lyricist with the composer, depending
//     on whether or not the composer's date are displayed.  For TiMP files, if there is
//     an editor, then show on the top left automatically.
//

std::string HumdrumInput::automaticHeaderLeft(std::vector<std::pair<std::string, std::string>> &biblist,
    std::map<std::string, std::string> &refmap, int linecount)
{
    std::string output;

    auto PTL = refmap.find("PTL");
    auto PPR = refmap.find("PTL");
    auto PPP = refmap.find("PTL");
    auto PDT = refmap.find("PTL");

    int count = 0;
    if (PTL != refmap.end()) {
        count++;
    }
    if (PPR != refmap.end()) {
        count++;
    }
    if (PPP != refmap.end()) {
        count++;
    }
    if (PDT != refmap.end()) {
        count++;
    }

    std::string person;
    if (count == 4) {
        auto PED = refmap.find("PED");
        if (PED != refmap.end()) {
            person = PED->second;
        }
    }
    else {
        auto itL = refmap.find("LYR");
        if (itL != refmap.end()) {
            person = itL->second;
        }
    }

    auto pos = person.find(",");
    if (pos != std::string::npos) {
        std::string lastname = person.substr(0, pos);
        std::string firstname = person.substr(pos + 1);
        person = firstname + " " + lastname;
    }

    if ((count != 4) && (!person.empty())) {
        auto itLD = refmap.find("LDT");
        if (itLD != refmap.end()) {
            person += " (" + itLD->second + ")";
        }
    }
    else if ((count == 4) && (!person.empty())) {
        person += ", <rend fontstyle=\"italic\">ed.</rend>";
    }

    if (!person.empty()) {
        output += "<rend fontsize=\"small\" halign=\"left\" valign=\"bottom\">";
        output += unescapeHtmlEntities(person);
        output += "</rend>\n";
        if (linecount > 1) {
            output += "<rend fontsize=\"small\" halign=\"left\" valign=\"bottom\">";
            output += "&#160;";
            output += "</rend>\n";
        }
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::automaticHeaderRight -- If there is no reference record "header-right",
//     Then the "COM" (composer) will be displayed.  The full name will be displayed,
//     and if the ordering of the name is "last, first" then it will be reversed to
//     "first last".   If "CDT" is found (composer's dates), then it will be added
//     underneath the composer's name.
//     The linecount variable is used to align the lyricist with the composer, depending
//     on whether or not the composer's date are displayed.
//

std::string HumdrumInput::automaticHeaderRight(std::vector<std::pair<std::string, std::string>> &biblist,
    std::map<std::string, std::string> &refmap, int &linecount)
{

    linecount = 0;
    std::string output;

    std::string composer;
    auto it2 = refmap.find("COM");
    if (it2 != refmap.end()) {
        std::string name = it2->second;
        auto pos = name.find(",");
        if (pos == std::string::npos) {
            composer = name;
        }
        else {
            std::string lastname = name.substr(0, pos);
            std::string firstname = name.substr(pos + 1);
            composer = firstname + " " + lastname;
        }
    }
    // <rend fontsize="small" halign="right" valign="bottom">Composer's name</rend>
    if (!composer.empty()) {
        output += "<rend fontsize=\"small\" halign=\"right\" valign=\"bottom\">";
        output += unescapeHtmlEntities(composer);
        output += "</rend>\n";
        linecount++;
    }
    // Composer's dates
    std::string outputdate = processReferenceTemplate("@{CDT:y}", biblist, refmap);

    if (!outputdate.empty()) {
        output += "<rend fontsize=\"small\" halign=\"right\" valign=\"bottom\">";
        output += outputdate;
        output += "</rend>\n";
        linecount++;
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::automaticHeaderCenter --
//

std::string HumdrumInput::automaticHeaderCenter(
    std::vector<std::pair<std::string, std::string>> &biblist, std::map<std::string, std::string> &refmap)
{
    std::string output;
    std::string title;

    int counter = 0;
    auto it = refmap.find("OTL");
    if (it != refmap.end()) {
        title = it->second;
    }

    std::string PTL; // publication title
    std::string PPR; // publisher
    std::string PPP; // publisher place
    std::string PDT; // publication date
    std::string PUBformat; // publication format

    it = refmap.find("PTL");
    if (it != refmap.end()) {
        PTL = it->second;
        counter++;
    }

    it = refmap.find("PPR");
    if (it != refmap.end()) {
        PPR = it->second;
        counter++;
    }

    it = refmap.find("PPP");
    if (it != refmap.end()) {
        PPP = it->second;
        counter++;
    }

    it = refmap.find("PDT");
    if (it != refmap.end()) {
        PDT = it->second;
        counter++;
    }

    it = refmap.find("PUB-format");
    if (it != refmap.end()) {
        PUBformat = it->second;
    }

    std::string subtitle;
    if (counter == 4) {
        if (!PUBformat.empty()) {
            subtitle += "in ";
        }
        std::string outputdate = processReferenceTemplate("@{PDT:y}", biblist, refmap);
        subtitle += "<rend fontstyle=\"italic\">";
        subtitle += PTL;
        subtitle += "</rend>";
        subtitle += " (";
        subtitle += PPP;
        subtitle += ": ";
        subtitle += PPR;
        if (!outputdate.empty()) {
            subtitle += ", ";
            subtitle += outputdate;
        }
        subtitle += ")";
    }

    if (title.empty()) {
        for (int i = 0; i < (int)biblist.size(); ++i) {
            if (biblist[i].first.substr(0, 3) == "OTL") {
                title = biblist[i].second;
                break;
            }
        }
    }

    // Special code for Tasso in Music Project: Add rime number after title.
    std::string rime;
    auto itrime = refmap.find("rime");
    if (itrime != refmap.end()) {
        rime = itrime->second;
    }
    // <rend fontsize="x-large">Non è questa la mano (<rend fontstyle="italic">Rime</rend> 47)</rend>
    if (!title.empty()) {
        output += "<rend halign=\"center\" valign=\"middle\">\n";
        output += "   <rend fontsize=\"large\">";
        output += unescapeHtmlEntities(title);
        if (!rime.empty()) {
            output += " (<rend fontstyle=\"italic\">Rime</rend>&#160;";
            output += rime;
            output += ")";
        }
        output += "</rend>\n";
        if (!subtitle.empty()) {
            output += "<lb/>";
            output += "<rend fontsize=\"small\">";
            output += subtitle;
            output += "</rend>";
        }
        output += "<lb/>&#160;\n<lb/>&#160;\n";
        output += "</rend>\n";
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::getStaffNumberLabel -- Return number 13 in pattern *staff13.
//

int HumdrumInput::getStaffNumberLabel(hum::HTp spinestart)
{
    hum::HTp tok = spinestart;
    while (tok) {
        if (tok->isData()) {
            break;
        }
        if (!tok->isInterpretation()) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->compare(0, 6, "*staff") != 0) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->size() <= 6) {
            tok = tok->getNextToken();
            continue;
        }
        std::string number = tok->substr(6, std::string::npos);
        if (!std::isdigit(number[0])) {
            tok = tok->getNextToken();
            continue;
        }
        return stoi(number);
    }
    return 0;
}

//////////////////////////////
//
// HumdrumInput::getPartNumberLabel -- Return number 2 in pattern *part2.
//

int HumdrumInput::getPartNumberLabel(hum::HTp spinestart)
{
    hum::HTp tok = spinestart;
    while (tok) {
        if (tok->isData()) {
            break;
        }
        if (!tok->isInterpretation()) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->compare(0, 5, "*part") != 0) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->size() <= 5) {
            tok = tok->getNextToken();
            continue;
        }
        std::string number = tok->substr(5, std::string::npos);
        if (!std::isdigit(number[0])) {
            tok = tok->getNextToken();
            continue;
        }
        return stoi(number);
    }
    return 0;
}

//////////////////////////////
//
// HumdrumInput::getGroupNumberLabel -- Return number 7 in pattern *group7.
//

int HumdrumInput::getGroupNumberLabel(hum::HTp spinestart)
{
    hum::HTp tok = spinestart;
    while (tok) {
        if (tok->isData()) {
            break;
        }
        if (!tok->isInterpretation()) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->compare(0, 6, "*group") != 0) {
            tok = tok->getNextToken();
            continue;
        }
        if (tok->size() <= 6) {
            tok = tok->getNextToken();
            continue;
        }
        std::string number = tok->substr(6, std::string::npos);
        if (!std::isdigit(number[0])) {
            tok = tok->getNextToken();
            continue;
        }
        return stoi(number);
    }
    return 0;
}

//////////////////////////////
//
// HumdrumInput::getSystemDecoration --
//

string HumdrumInput::getSystemDecoration(const std::string &tag)
{
    hum::HumdrumFile &infile = m_infiles[0];
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isReference()) {
            continue;
        }
        std::string key = infile[i].getReferenceKey();
        if (key != tag) {
            continue;
        }
        std::string value = infile[i].getReferenceValue();
        std::string output;
        for (int j = 0; j < (int)value.size(); ++j) {
            if (std::isspace(value[j])) {
                continue;
            }
            output.push_back(value[j]);
        }
        return output;
    }
    return "";
}

//////////////////////////////
//
// HumdrumInput::addMidiTempo --
//

void HumdrumInput::addMidiTempo(ScoreDef *scoreDef, hum::HTp kernpart, int top, int bot)
{
    if (top <= 0) {
        top = 4;
    }
    if (bot <= 0) {
        bot = 4;
    }
    if (m_mens) {
        bot = 1;
    }
    bool foundtempo = false;
    while (kernpart != NULL) {
        if (kernpart->isData()) {
            break;
        }
        if (!kernpart->isInterpretation()) {
            kernpart = kernpart->getNextToken();
            continue;
        }
        if (kernpart->compare(0, 3, "*MM") == 0) {
            if (kernpart->size() > 3) {
                if (::isdigit((*kernpart)[3])) {
                    int tempo = stoi(kernpart->substr(3));
                    // std::string tempostr = to_string(tempo);
                    scoreDef->SetMidiBpm(tempo * m_globalTempoScaling * m_localTempoScaling.getFloat());
                    foundtempo = true;
                }
            }
            break;
        }
        kernpart = kernpart->getNextToken();
    }
    if (!foundtempo) {
        hum::HumdrumFile &infile = *(kernpart->getOwner()->getOwner());
        hum::HumRegex hre;
        hum::HTp omd = NULL;
        for (int i = 0; i < infile.getLineCount(); ++i) {
            if (infile[i].isData()) {
                break;
            }
            hum::HTp token = infile[i].token(0);
            if (hre.search(token, "!!!OMD")) {
                omd = token;
                // Don't break: search for last OMD in non-data region.
            }
        }
        if (omd) {
            int guess = hum::Convert::tempoNameToMm(*omd, bot, top);
            if (guess > 0) {
                scoreDef->SetMidiBpm(guess * m_globalTempoScaling * m_localTempoScaling.getFloat());
            }
            else {
                addDefaultTempo(scoreDef);
            }
        }
        else {
            addDefaultTempo(scoreDef);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::addDefaultTempo --  Add MM400 if average rhythm is more than
//    a half note (for basic Renaissance default tempo).
//

void HumdrumInput::addDefaultTempo(ScoreDef *scoreDef)
{
    if (m_mens) {
        scoreDef->SetMidiBpm(400.0 * m_globalTempoScaling);
        return;
    }
    double sum = 0.0;
    int count = 0;
    hum::HumdrumFile &infile = m_infiles[0];
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (infile[i].getDuration() == 0) {
            continue;
        }
        count++;
        sum += infile[i].getDuration().getFloat();
    }
    double avgdur = sum / count;
    if (avgdur > 2.0) {
        scoreDef->SetMidiBpm(400.0 * m_globalTempoScaling);
    }
    else if (m_globalTempoScaling != 1.0) {
        scoreDef->SetMidiBpm(120.0 * m_globalTempoScaling);
    }
}

//////////////////////////////
//
// HumdrumInput::fillStaffInfo -- Should use regular expressions
//    in the future.
//

void HumdrumInput::fillStaffInfo(hum::HTp staffstart, int staffnumber, int staffcount)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    std::string primarymensuration;

    int group = getGroupNumberLabel(staffstart);
    int staffindex = staffnumber - 1;

    // bool hasglabel = false;
    std::string glabel;
    hum::HTp glabeltok = NULL;
    std::string gabbreviation;
    hum::HTp gabbrtok = NULL;

    bool haslabel = false;
    std::string label;
    hum::HTp labeltok = NULL;
    std::string abbreviation;
    hum::HTp abbrtok = NULL;
    hum::HTp stafftok = NULL;
    hum::HTp parttok = NULL;

    std::string stria; // number of staff lines
    hum::HTp striatok = NULL;
    std::string clef;
    hum::HTp cleftok = NULL;
    hum::HTp keytok = NULL;
    std::string keysig;
    hum::HTp keysigtok = NULL;
    std::string key;
    hum::HTp icode = NULL;
    std::string transpose;
    std::string itranspose;
    hum::HTp staffscale = NULL;
    std::string timesig;
    hum::HTp timetok = NULL;
    std::string metersig;
    hum::HTp metertok = NULL;
    int top = 0;
    int bot = 0;
    bool manip = false;

    hum::HumRegex hre;
    hum::HTp part = staffstart;
    while (part && !part->getLine()->isData()) {
        if (*part == "*^") {
            manip = true;
        }
        if (part->compare(0, 5, "*clef") == 0) {
            if (cleftok) {
                if (clef == *part) {
                    // there is already a clef found, and it is the same
                    // as this one, so ignore the second one.
                }
                else {
                    // mark clef as a clef change to print in the layer
                    part->setValue("auto", "clefChange", 1);
                    markOtherClefsAsChange(part);
                }
                part = part->getNextToken();
                continue;
            }

            if (hre.search(part, 5, "\\d")) {
                clef = *part;
                cleftok = part;
            }
            else if (part->find("clefX") != std::string::npos) {
                // allow percussion clef to not have a line number since it is unpitched.
                clef = *part;
                cleftok = part;
            }
        }
        else if (part->compare(0, 6, "*oclef") == 0) {
            if (hre.search(part, 6, "\\d")) {
                m_oclef.emplace_back(staffnumber, part);
                if (part->isMensLike()) {
                    // Override *clef with *oclef if displaying **mens:
                    clef = *part;
                    cleftok = part;
                }
            }
        }
        else if (part->compare(0, 5, "*part") == 0) {
            parttok = part;
        }
        else if (part->compare(0, 6, "*staff") == 0) {
            stafftok = part;
        }
        else if (part->compare(0, 6, "*stria") == 0) {
            stria = *part;
            striatok = part;
        }
        else if (part->compare(0, 5, "*omet") == 0) {
            m_omet.emplace_back(staffnumber, part);
        }
        else if (part->compare(0, 3, "*k[") == 0) {
            keysigtok = part;
            keysig = *keysigtok;
        }
        else if (part->compare(0, 4, "*ok[") == 0) {
            m_okey.emplace_back(staffnumber, part);
        }
        else if (hre.search(part, "^\\*[a-gA-G][#-]*:([a-z]{3})?$")) {
            keytok = part;
        }
        else if (part->compare(0, 7, "*scale:") == 0) {
            staffscale = part;
        }
        else if (part->compare(0, 6, "*size:") == 0) {
            staffscale = part;
        }
        else if (part->compare(0, 4, "*Trd") == 0) {
            transpose = *part;
        }
        else if (part->compare(0, 5, "*ITrd") == 0) {
            itranspose = *part;
        }
        else if ((part->compare(0, 4, "*I''") == 0) && !manip) {
            if (staffcount > 1) {
                // Avoid encoding the part group abbreviation when there is only one
                // part in order to suppress the display of the abbreviation.
                gabbreviation = part->substr(4);
                gabbrtok = part;
                if ((group > 0) && !gabbreviation.empty()) {
                    m_group_abbr[group] = gabbreviation;
                    m_group_abbr_tok[group] = gabbrtok;
                }
            }
        }
        else if ((part->compare(0, 3, "*I'") == 0) && !manip) {
            if (staffcount > 1) {
                // Avoid encoding the part abbreviation when there is only one
                // part in order to suppress the display of the abbreviation.
                abbreviation = part->substr(3);
                abbrtok = part;
            }
        }
        else if ((part->compare(0, 4, "*I\"\"") == 0) && !manip) {
            glabel = part->substr(4);
            glabeltok = part;
            // hasglabel = true;
            if ((group > 0) && !glabel.empty()) {
                m_group_name[group] = glabel;
                m_group_name_tok[group] = glabeltok;
            }
        }
        else if ((part->compare(0, 3, "*I\"") == 0) && !manip) {
            label = part->substr(3);
            labeltok = part;
            haslabel = true;
        }
        else if ((part->compare(0, 2, "*I") == 0) && !manip) {
            // check to see if an instrument code
            int len = (int)part->size();
            if ((len > 2) && ::islower(part->at(2))) {
                icode = part;
                haslabel = true;
            }
        }
        else if (part->compare(0, 5, "*met(") == 0) {
            auto ploc = part->rfind(")");
            if (ploc != std::string::npos) {
                metersig = part->substr(5, ploc - 5);
                metertok = part;
            }
        }
        else if (sscanf(part->c_str(), "*M%d/%d", &top, &bot) == 2) {
            timesig = *part;
            timetok = part;
            ss[staffindex].meter_bottom = bot;
            ss[staffindex].meter_top = top;
            if (bot == 0) {
                // Can't to breve meters, so switch to semibreve meter (whole notes).
                ss[staffindex].meter_bottom = 1;
                ss[staffindex].meter_top *= 2;
            }
        }
        else if (part->find("acclev") != std::string::npos) {
            storeAcclev(*part, staffindex);
        }
        else if (part->compare(0, 5, "*stem") == 0) {
            storeStemInterpretation(*part, staffindex, part->getSubtrack());
        }
        else if (part->compare(0, 6, "*Xstem") == 0) {
            storeStemInterpretation(*part, staffindex, part->getSubtrack());
        }

        hum::HumdrumFile *hf = part->getOwner()->getOwner();
        int line = part->getLineIndex();
        for (int ii = line + 1; ii < hf->getLineCount(); ++ii) {
            if ((*hf)[ii].isGlobalComment()) {
                if ((*hf)[ii].compare(0, 22, "!!primary-mensuration:") == 0) {
                    std::string pmen = (*hf)[ii];
                    auto ploc1 = pmen.find("met(");
                    if (ploc1 != std::string::npos) {
                        auto ploc2 = pmen.rfind(")");
                        if (ploc2 != std::string::npos) {
                            primarymensuration = pmen.substr(ploc1 + 4, ploc2 - ploc1 - 4);
                        }
                    }
                }
            }
            else {
                break;
            }
        }

        part = part->getNextToken();
    }

    if (labeltok == NULL) {
        // try to infer the name of the part automatically
        if (icode) {
            label = getLabelFromInstrumentCode(icode, itranspose);
        }
    }

    // short-circuit *clef with *oclef for **mens data
    if (staffstart->isMensLike()) {
        if ((!m_oclef.empty()) && (staffnumber == m_oclef.back().first)) {
            clef = *m_oclef.back().second;
            cleftok = m_oclef.back().second;
        }
    }

    // short-circuit *met with *omet for **mens data
    if (staffstart->isMensLike()) {
        if ((!m_omet.empty()) && (staffnumber == m_omet.back().first)) {
            metersig = *m_omet.back().second;
            metertok = m_omet.back().second;
        }
    }

    m_staffdef.back()->SetN(staffnumber);

    if (stafftok) {
        // search for a **dynam before the next **kern spine, and set the
        // dynamics position to centered if there is a slash in the *staff1/2 string.
        // In the future also check *part# to see if there are two staves for a part
        // with no **dynam for the lower staff (infer to be a grand staff).
        hum::HTp dynamspine = getAssociatedDynamSpine(stafftok);
        if (dynamspine != NULL) {
            if (dynamspine->compare(0, 6, "*staff") == 0) {
                if (dynamspine->find('/') != std::string::npos) {
                    // the dynamics should be placed between
                    // staves: the current one and the one below it.
                    ss.at(staffindex).m_dynampos = 0;
                    ss.at(staffindex).m_dynamstaffadj = 0;
                    ss.at(staffindex).m_dynamposdefined = true;
                }
            }
        }
    }
    if (parttok) {
        hum::HTp dynamspine = getAssociatedDynamSpine(parttok);
        int partnum = 0;
        int dpartnum = 0;
        int lpartnum = 0;
        hum::HumRegex hre;

        if (dynamspine) {
            if (hre.search(dynamspine, "^\\*part(\\d+)")) {
                dpartnum = hre.getMatchInt(1);
            }
        }
        if (dpartnum > 0) {
            if (hre.search(parttok, "^\\*part(\\d+)")) {
                partnum = hre.getMatchInt(1);
            }
        }
        if (partnum > 0) {
            hum::HTp lspine = getPreviousStaffToken(parttok);
            if (lspine) {
                if (hre.search(lspine, "^\\*part(\\d+)")) {
                    lpartnum = hre.getMatchInt(1);
                }
            }
        }
        if (lpartnum > 0) {
            if ((lpartnum == partnum) && (dpartnum == partnum)) {
                ss.at(staffindex).m_dynampos = 0;
                ss.at(staffindex).m_dynamstaffadj = 0;
                ss.at(staffindex).m_dynamposdefined = true;
            }
        }
    }

    if (staffscale != NULL) {
        auto loc = staffscale->find(":");
        if (loc != std::string::npos) {
            std::string value = staffscale->substr(loc + 1);
            if (!value.empty()) {
                if (value.back() == '%') {
                    m_staffdef.back()->SetScale(m_staffdef.back()->AttScalable::StrToPercent(value));
                }
            }
        }
    }

    if (!stria.empty()) {
        hum::HumRegex hre;
        if (hre.search(stria, "^\\*stria(\\d+)")) {
            int number = hre.getMatchInt(1);
            if (number < 33) {
                // Just to avoid cases where the staff number is something like 14532
                m_staffdef.back()->SetLines(number);
            }
            else {
                m_staffdef.back()->SetLines(5);
            }
        }
        else {
            m_staffdef.back()->SetLines(5);
        }
    }
    else {
        m_staffdef.back()->SetLines(5);
    }

    if (clef.size() > 0) {
        setClef(m_staffdef.back(), clef, cleftok, striatok);
        ss.at(staffindex).last_clef = clef;
    }
    else {
        std::string autoclef = getAutoClef(staffstart, staffnumber);
        setClef(m_staffdef.back(), autoclef, NULL);
        ss.at(staffindex).last_clef = clef;
    }

    if (transpose.size() > 0) {
        setTransposition(m_staffdef.back(), transpose);
    }

    if (itranspose.size() > 0) {
        // This has to be above setKeySig():
        setDynamicTransposition(staffindex, m_staffdef.back(), itranspose);
    }

    if (abbreviation.size() > 0) {
        setInstrumentAbbreviation(m_staffdef.back(), abbreviation, abbrtok);
    }

    if (haslabel) {
        if ((labeltok == NULL) && !icode->empty()) {
            setInstrumentName(m_staffdef.back(), label, icode);
        }
        else if (hasIndent(labeltok)) {
            setInstrumentName(m_staffdef.back(), "   ");
        }
        else {
            setInstrumentName(m_staffdef.back(), label, labeltok);
        }
    }
    else if (staffnumber == 1) {
        if (hasIndent(staffstart)) {
            setInstrumentName(m_staffdef.back(), "   ");
        }
        // setInstrumentName(m_staffdef.back(), "&#160;&#160;&#160;");
        // setInstrumentName(m_staffdef.back(), "\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0");
    }
    else if (hasIndent(staffstart)) {
        setInstrumentName(m_staffdef.back(), "   ");
    }

    if (keysig.size() > 0) {
        setKeySig(staffindex, m_staffdef.back(), keysig, keysigtok, keytok, false);
    }

    if (primarymensuration.empty()) {
        if (timesig.size() > 0) {
            setTimeSig(m_staffdef.back(), timesig, metersig, staffstart, timetok, metertok);
        }
        if (!m_mens && (metersig.size() > 0)) {
            setMeterSymbol(m_staffdef.back(), metersig, staffindex, staffstart, metertok);
        }
    }
    else {
        if ((primarymensuration == "C|") || (primarymensuration == "c|")) {
            setTimeSig(m_staffdef.back(), "*M2/1", metersig, staffstart);
            setMeterSymbol(m_staffdef.back(), primarymensuration, staffindex, staffstart, metertok);
        }
        else if ((primarymensuration == "C") || (primarymensuration == "c")) {
            setTimeSig(m_staffdef.back(), "*M4/1", metersig, staffstart, metertok);
            setMeterSymbol(m_staffdef.back(), primarymensuration, staffindex, staffstart);
        }
        else if ((primarymensuration == "O") || (primarymensuration == "o")) {
            setTimeSig(m_staffdef.back(), "*M3/1", metersig, staffstart, metertok);
            setMeterSymbol(m_staffdef.back(), primarymensuration, staffindex, staffstart);
        }
    }

    addInstrumentDefinition(m_staffdef.back(), staffstart);

    if (staffstart->isMensLike()) {
        if (isBlackNotation(staffstart)) {
            m_staffdef.back()->SetNotationtype(NOTATIONTYPE_mensural_black);
            ss.at(staffindex).mensuration_type = 1;
        }
        else {
            m_staffdef.back()->SetNotationtype(NOTATIONTYPE_mensural_white);
            ss.at(staffindex).mensuration_type = 0;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getAssociatedDynamSpine -- Return the first **dynam
//     spine before another staff spine is found; or return NULL token
//     first;
//

hum::HTp HumdrumInput::getAssociatedDynamSpine(hum::HTp stafftok)
{
    if (!stafftok) {
        return NULL;
    }
    hum::HTp current = stafftok;
    current = current->getNextFieldToken();
    while (current) {
        if (current->isStaff()) {
            break;
        }
        if (current->isDataType("**dynam")) {
            return current;
        }
        current = current->getNextFieldToken();
    }
    return NULL;
}

//////////////////////////////
//
// HumdrumInput::getPreviousStaffToken -- return the first staff token
//    to the left which is not the same track as the current token, and
//    also is the first subspine of that track.  Return NULL if no previous
//    staff token.
//

hum::HTp HumdrumInput::getPreviousStaffToken(hum::HTp parttok)
{
    if (!parttok) {
        return NULL;
    }
    int track = parttok->getTrack();
    int ttrack = -1;
    hum::HTp current = parttok->getPreviousFieldToken();
    while (current) {
        if (!current->isStaff()) {
            current = current->getPreviousFieldToken();
            continue;
        }
        ttrack = current->getTrack();
        if (ttrack == track) {
            current = current->getPreviousFieldToken();
            continue;
        }
        break;
    }
    if (!current) {
        return NULL;
    }
    track = ttrack;
    hum::HTp lastc = current;
    current = current->getPreviousFieldToken();
    while (current) {
        ttrack = current->getTrack();
        if (ttrack == track) {
            lastc = current;
            current = current->getPreviousFieldToken();
        }
        break;
    }

    return lastc;
}

//////////////////////////////
//
// HumdrumInput::isBlackNotation --
//

bool HumdrumInput::isBlackNotation(hum::HTp starting)
{
    hum::HTp current = starting;
    while (current && !current->isData()) {
        if (current->isInterpretation()) {
            if (*current == "*black") {
                return true;
            }
        }
        current = current->getNextToken();
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::getLabelFromInstrumentCode -- Can be expanded to output
//   instrument names in a specific language.
//   Example: *Iclars  *ITrd1c2  == Clarinet in B-flat
//

std::string HumdrumInput::getLabelFromInstrumentCode(hum::HTp icode, const std::string &transpose)
{
    std::string output;
    std::string name = icode->substr(2);

    if (name == "piano") {
        output = "Piano";
    }
    else if (name == "flt") {
        output = "Flute";
    }
    else if (name == "picco") {
        output = "Piccolo";
    }
    else if (name == "oboe") {
        output = "Oboe";
    }
    else if (name == "clars") {
        output = "Clarinet";
    }
    else if (name == "clara") {
        output = "Alto Clarinet";
    }
    else if (name == "clarb") {
        output = "Bass Clarinet";
    }
    else if (name == "fagot") {
        output = "Bassoon";
    }
    else if (name == "fagot") {
        output = "Bassoon";
    }
    else if (name == "tromp") {
        output = "Trumpet";
    }
    else if (name == "tromb") {
        output = "Trombone";
    }
    else if (name == "violin") {
        // Deal with Violin 1 versus Violin 2, but need more info to do that.
        output = "Violin";
    }
    else if (name == "viola") {
        output = "Viola";
    }
    else if (name == "cello") {
        output = "Violoncello";
    }
    else if (name == "cemba") {
        output = "Harpsichord";
    }
    else if (name == "organ") {
        output = "Organ";
    }
    else if (name == "clavi") {
        output = "Clavichord";
    }
    else if (name == "forte") {
        output = "Fortepiano";
    }
    else if (name == "guitr") {
        output = "Guitar";
    }
    else if (name == "cbass") {
        output = "Contrabass";
    }
    else if (name == "koto") {
        output = "Koto";
    }

    if (output.empty()) {
        // could not find an automatic name for the instrument.
        return output;
    }

    // Some instruments always have a specific key, such as
    // alto flute, which is in G, but the transposition is
    // often not given in the name.
    if (transpose == "*ITrd1c2") {
        output += " in B-flat";
    }
    else if (transpose == "*ITrd2c3") {
        output += " in A";
    }
    else if (transpose == "*ITrd-2c-3") {
        output += " in E-flat";
    }
    else if (transpose == "*ITrd-5c-9") {
        output += " in E-flat";
    }

    return output;
}

//////////////////////////////
//
// hasIndent -- true if *indent tandem interpretation before first data token.
//

bool HumdrumInput::hasIndent(hum::HTp tok)
{
    hum::HTp current = tok;
    while (current) {
        if (!current->isInterpretation()) {
            current = current->getNextToken();
            continue;
        }
        if (*current == "*indent") {
            return true;
        }
        if (*current == "*I\"") {
            // alternate version (empty part name)
            return true;
        }
        current = current->getNextToken();
        continue;
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::setInstrumentName -- for staffDef or staffGrp.
//

template <class ELEMENT>
void HumdrumInput::setInstrumentName(ELEMENT *element, const std::string &name, hum::HTp labeltok)
{
    if (name.empty()) {
        // no instrument name to display
        return;
    }
    Label *label = new Label();
    if (name == "   ") {
        Text *text = new Text();
        text->SetText(U"\u00a0\u00a0\u00a0");
        label->AddChild(text);
    }
    else {
        setLabelContent(label, name);
    }
    if (labeltok) {
        setLocationId(label, labeltok);
    }
    element->InsertChild(label, 0);
}

//////////////////////////////
//
// HumdrumInput::setInstrumentAbbreviation -- for staffDef or staffGrp.
//

template <class ELEMENT>
void HumdrumInput::setInstrumentAbbreviation(ELEMENT *element, const std::string &name, hum::HTp abbrtok)
{
    if (name.empty()) {
        return;
    }
    LabelAbbr *label = new LabelAbbr();
    Text *text = new Text();
    if (abbrtok) {
        setLocationId(label, abbrtok);
    }

    std::string name8 = name;
    // Substitute b and "-flat" for Unicode flat symbol:
    std::regex exp1("\\b([ABDEFG])b\\b");
    std::regex exp2("\\b([A-Ga-g])-flat\\b");
    name8 = std::regex_replace(name8, exp1, "$1\xe2\x99\xad");
    name8 = std::regex_replace(name8, exp2, "$1\xe2\x99\xad");
    // Substitute # and "-sharp" for Unicode sharp symbol:
    std::regex exp3("\\b([A-G])#\\b");
    std::regex exp4("\\b([A-Ga-g])-sharp\\b");
    name8 = std::regex_replace(name8, exp3, "$1\xe2\x99\xaf");
    name8 = std::regex_replace(name8, exp4, "$1\xe2\x99\xaf");

    std::u32string name16 = UTF8to32(name8);
    text->SetText(name16);
    label->AddChild(text);
    element->InsertChild(label, 0);
}

//////////////////////////////
//
// HumdrumInput::getAutoClef -- estimate a clef for a part
//     that does not have a specified clef.  Choice will be
//     treble or bass.
//

string HumdrumInput::getAutoClef(hum::HTp partstart, int partnumber)
{
    hum::HTp tok = partstart;
    int ptrack = partstart->getTrack();
    std::vector<int> dhist(100, 0);
    int diatonic;
    hum::HumRegex hre;
    while (tok) {
        if (tok->isInterpretation()) {
            if (tok->compare(0, 5, "*clef") == 0) {
                if (hre.search(tok, 5, "\\d")) {
                    break;
                }
            }
        }
        if (!tok->isData()) {
            tok = tok->getNextToken();
            continue;
        }
        if (!(tok->isNull() || tok->isRest())) {
            diatonic = hum::Convert::kernToBase7(tok);
            if ((diatonic > 0) && (diatonic < 100)) {
                dhist[diatonic]++;
            }
        }
        hum::HTp ftok = tok->getNextFieldToken();
        while (ftok && (ptrack == ftok->getTrack())) {
            if (!(ftok->isNull() || ftok->isRest())) {
                diatonic = hum::Convert::kernToBase7(ftok);
                if ((diatonic > 0) && (diatonic < 100)) {
                    dhist[diatonic]++;
                }
            }
            ftok = ftok->getNextFieldToken();
        }

        tok = tok->getNextToken();
    }

    int low = 100;
    int high = 0;
    for (int i = 0; i < 100; ++i) {
        if (dhist[i]) {
            low = i;
            break;
        }
    }
    for (int i = 99; i >= 0; i--) {
        if (dhist[i]) {
            high = i;
            break;
        }
    }
    if (high < low) {
        return "";
    }
    int E3 = hum::Convert::kernToBase7("E");
    int G4 = hum::Convert::kernToBase7("g");
    if ((low > E3) && (high > G4)) {
        return "clefG2";
    }
    if ((low < E3) && (high < G4)) {
        return "clefF4";
    }
    // calculate weighted mean
    int wsum = 0;
    for (int i = low; i <= high; ++i) {
        wsum += dhist[i];
    }
    if (wsum == 0) {
        // no pitches...
        return "";
    }
    int topsum = 0;
    for (int i = low; i <= high; ++i) {
        topsum += i * wsum;
    }
    int C4 = hum::Convert::kernToBase7("c");
    double wm = (double)topsum / (double)wsum;
    if (wm < C4) {
        return "clefF4";
    }
    else {
        return "clefG2";
    }
}

//////////////////////////////
//
// HumdrumInput::addInstrumentDefinition --
//
// <staffDef>
//    <instrDef @midi.channel @midi.instrnum @midi.instrname>
//

void HumdrumInput::addInstrumentDefinition(StaffDef *staffdef, hum::HTp partstart)
{

    hum::HTp instcode = NULL;
    while (partstart != NULL) {
        if (partstart->isData()) {
            break;
            ;
        }
        if (!partstart->isInterpretation()) {
            partstart = partstart->getNextToken();
            continue;
        }
        if (partstart->compare(0, 2, "*I") == 0) {
            if (partstart->size() < 2) {
                partstart = partstart->getNextToken();
                continue;
            }
            if (!::islower((*partstart)[2])) {
                // Instrument class, name, abbrevation or similar.
                // The first letter of the instrument code must
                // be a lowercase letter, but it is not, so keep
                // searching.
                partstart = partstart->getNextToken();
                continue;
            }
            instcode = partstart;
            break;
        }
        partstart = partstart->getNextToken();
    }

    if (instcode == NULL) {
        return;
    }

    // Allowing users to assign MIDI instrument numbers in data would be useful, but
    // currently only allowed via insturment codes.
    static hum::HumInstrument imap;
    int gmpc = imap.getGM(*instcode);

    //   gmpc is -1 if no mapping, so don't add General MIDI insturment number in that case
    if (gmpc < 0) {
        return;
    }

    InstrDef *idef = new InstrDef();
    staffdef->AddChild(idef);
    int offset = 0; // 1 in MEI 3, and 0 in MEI 4
    idef->SetMidiInstrnum(gmpc + offset);
    data_MIDINAMES idval = (data_MIDINAMES)(gmpc + 1);
    idef->SetMidiInstrname(idval);
}

//////////////////////////////
//
// HumdrumInput::setMeterSymbol -- common time or cut time for CMN.
//

template <class ELEMENT>
void HumdrumInput::setMeterSymbol(
    ELEMENT *element, const std::string &metersig, int staffindex, hum::HTp partstart, hum::HTp metertok)
{
    if ((partstart != NULL) && partstart->isMensLike()) {
        setMensurationSymbol(element, metersig, staffindex, metertok);
        return;
    }

    // handle mensuration displays:
    if (metersig.find("C") != std::string::npos) {
        setMensurationSymbol(element, metersig, staffindex, metertok);
        return;
    }
    if (metersig.find("O") != std::string::npos) {
        setMensurationSymbol(element, metersig, staffindex, metertok);
        return;
    }

    MeterSig *vrvmetersig = getMeterSig(element);
    if (!vrvmetersig) {
        return;
    }

    if (metersig == "C") {
        // This is used more strictly for C mensuration.
        vrvmetersig->SetSym(METERSIGN_common);
    }
    else if (metersig == "c") {
        vrvmetersig->SetSym(METERSIGN_common);
    }
    else if (metersig == "c|") {
        vrvmetersig->SetSym(METERSIGN_cut);
    }
    else if (metersig == "C|") {
        // This is used more strictly for Cut-C mensuration.
        vrvmetersig->SetSym(METERSIGN_cut);
    }
    else if (metersig == "*omet(C)") {
        // This is used more strictly for C mensuration.
        vrvmetersig->SetSym(METERSIGN_common);
    }
    else if (metersig == "*omet(c)") {
        vrvmetersig->SetSym(METERSIGN_common);
    }
    else if (metersig == "*omet(c|)") {
        vrvmetersig->SetSym(METERSIGN_cut);
    }
    else if (metersig == "*omet(C|)") {
        // This is used more strictly for Cut-C mensuration.
        vrvmetersig->SetSym(METERSIGN_cut);
    }
}

//////////////////////////////
//
// HumdrumInput::setMensurationSymbol --
//

template <class ELEMENT>
void HumdrumInput::setMensurationSymbol(
    ELEMENT *element, const std::string &metersig, int staffindex, hum::HTp mensurtok)
{
    int maximodus = 0;
    int modus = 0;
    int tempus = 0;
    int prolatio = 0;

    hum::HumRegex hre;
    Mensur *vrvmensur = getMensur(element, mensurtok);
    if (!vrvmensur) {
        return;
    }
    if (mensurtok) {
        setLocationId(vrvmensur, mensurtok);
    }
    if (metersig == "*met(C)" || metersig == "C") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        maximodus = 2;
        modus = 2;
        tempus = 2;
        prolatio = 2;
    }
    else if (metersig == "*met(C3)" || metersig == "C3") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        maximodus = 2;
        modus = 2;
        tempus = 3;
        prolatio = 2;
    }
    else if (metersig == "*met(C|)" || metersig == "C|") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        vrvmensur->SetSlash(1);
        maximodus = 2;
        modus = 2;
        tempus = 2;
        prolatio = 2;
    }
    else if (metersig == "*met(C|3)" || metersig == "C|3") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        vrvmensur->SetSlash(1);
        vrvmensur->SetNum(3);
        maximodus = 2;
        modus = 2;
        tempus = 2;
        prolatio = 2;
    }
    else if (metersig == "*met(O)" || metersig == "O") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        maximodus = 2;
        modus = 2;
        tempus = 3;
        prolatio = 2;
    }
    else if (metersig == "*met(O3)" || metersig == "O3") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        maximodus = 3;
        modus = 3;
        tempus = 3;
        prolatio = 2;
    }
    else if (metersig == "*met(O|)" || metersig == "O|") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        vrvmensur->SetSlash(1);
        maximodus = 2;
        modus = 2;
        tempus = 3;
        prolatio = 2;
    }
    else if (metersig == "*met(O|3)" || metersig == "O|3") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        vrvmensur->SetSlash(1);
        maximodus = 3;
        modus = 3;
        tempus = 3;
        prolatio = 2;
    }
    else if (metersig == "*met(O.)" || metersig == "O.") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_3);
        }
        maximodus = 2;
        modus = 2;
        tempus = 3;
        prolatio = 3;
    }
    else if (metersig == "*met(O.|)" || metersig == "O.|") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_3);
            vrvmensur->SetProlatio(PROLATIO_3);
        }
        vrvmensur->SetSlash(1);
        maximodus = 2;
        modus = 2;
        tempus = 3;
        prolatio = 3;
    }
    else if (metersig == "*met(C.)" || metersig == "C.") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_3);
        }
        maximodus = 2;
        modus = 2;
        tempus = 2;
        prolatio = 3;
    }
    else if (metersig == "*met(C.|)" || metersig == "C.|") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_3);
        }
        vrvmensur->SetSlash(1);
        maximodus = 2;
        modus = 2;
        tempus = 2;
        prolatio = 3;
    }
    else if (metersig == "*met(C|3/2)" || metersig == "C|3/2") {
        if (m_mens) {
            vrvmensur->SetTempus(TEMPUS_2);
            vrvmensur->SetProlatio(PROLATIO_2);
        }
        vrvmensur->SetNum(3);
        vrvmensur->SetNumbase(2);
        vrvmensur->SetSlash(1);
        tempus = 2;
        prolatio = 2;
    }

    if (metersig.find('C') != std::string::npos) {
        vrvmensur->SetSign(MENSURATIONSIGN_C);
        if (metersig.find("3/2") != std::string::npos) {
            vrvmensur->SetNum(3);
            vrvmensur->SetNumbase(2);
        }
        else if (metersig.find("C2") != std::string::npos) {
            vrvmensur->SetNum(2);
        }
        else if (metersig.find("C3") != std::string::npos) {
            vrvmensur->SetNum(3);
        }
    }
    else if (metersig.find('O') != std::string::npos) {
        vrvmensur->SetSign(MENSURATIONSIGN_O);
        if (metersig.find("3/2") != std::string::npos) {
            vrvmensur->SetNum(3);
            vrvmensur->SetNumbase(2);
        }
        else if (metersig.find("O2") != std::string::npos) {
            vrvmensur->SetNum(2);
        }
        else if (metersig.find("O3") != std::string::npos) {
            vrvmensur->SetNum(3);
        }
    }
    else {
        std::cerr << "Warning: do not understand mensuration " << metersig << std::endl;
        return;
    }

    if (metersig.find('|') != std::string::npos) {
        vrvmensur->SetSlash(1);
    }
    if (metersig.find('.') != std::string::npos) {
        vrvmensur->SetDot(BOOLEAN_true);
    }
    if (metersig.find('r') != std::string::npos) {
        vrvmensur->SetOrient(ORIENTATION_reversed);
    }

    if (hre.search(metersig, "(\\d+)/(\\d+)")) {
        vrvmensur->SetNum(hre.getMatchInt(1));
        vrvmensur->SetNumbase(hre.getMatchInt(2));
    }
    else if (hre.search(metersig, "/(\\d+)")) {
        vrvmensur->SetNumbase(hre.getMatchInt(1));
    }
    else if (hre.search(metersig, "(\\d+).*\\)")) {
        vrvmensur->SetNum(hre.getMatchInt(1));
    }

    if (!m_mens) {
        return;
    }

    // Set explicit rhymic level splits:
    // *met(C)_WXYZ
    // W = how many longs in one maxima
    // X = how many breves in one long
    // Y = how many semibreves in one breve
    // Z = how many minims in one semibreve
    // 0 can be used to specify and undefined value.
    if (mensurtok && hre.search(mensurtok, "_(\\d?)(\\d?)(\\d?)(\\d?)")) {
        std::string num1 = hre.getMatch(1);
        std::string num2 = hre.getMatch(2);
        std::string num3 = hre.getMatch(3);
        std::string num4 = hre.getMatch(4);

        if (!num1.empty()) {
            maximodus = stoi(num1);
        }
        if (!num2.empty()) {
            modus = stoi(num2);
        }
        if (!num3.empty()) {
            tempus = stoi(num3);
        }
        if (!num4.empty()) {
            prolatio = stoi(num4);
        }

        switch (prolatio) {
            case 2: vrvmensur->SetProlatio(PROLATIO_2); break;
            case 3: vrvmensur->SetProlatio(PROLATIO_3); break;
            case 0: break;
            default: cerr << "Warning: unknown prolation " << prolatio << " in " << mensurtok << endl;
        }
        switch (tempus) {
            case 2: vrvmensur->SetTempus(TEMPUS_2); break;
            case 3: vrvmensur->SetTempus(TEMPUS_3); break;
            case 0: break;
            default: cerr << "Warning: unknown tempus " << tempus << " in " << mensurtok << endl;
        }
        switch (modus) {
            case 2: vrvmensur->SetModusminor(MODUSMINOR_2); break;
            case 3: vrvmensur->SetModusminor(MODUSMINOR_3); break;
            case 0: break;
            default: cerr << "Warning: unknown modus " << modus << " in " << mensurtok << endl;
        }
        switch (maximodus) {
            case 2: vrvmensur->SetModusmaior(MODUSMAIOR_2); break;
            case 3: vrvmensur->SetModusmaior(MODUSMAIOR_3); break;
            case 0: break;
            default: cerr << "Warning: unknown maximodus " << maximodus << " in " << mensurtok << endl;
        }
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if (staffindex < 0) {
        cerr << "Initialization problem, not setting mensuration information" << endl;
        cerr << "STAFF INDEX = " << staffindex << endl;
        return;
    }
    if (staffindex >= (int)ss.size()) {
        cerr << "Problem with staff indexing in mensuration processing" << endl;
        return;
    }

    ss.at(staffindex).maximodus = maximodus;
    ss.at(staffindex).modus = modus;
    ss.at(staffindex).tempus = tempus;
    ss.at(staffindex).prolatio = prolatio;
}

//////////////////////////////
//
// HumdrumInput::setTimeSig -- Convert a Humdrum timesig to an MEI timesig.
//

void HumdrumInput::setTimeSig(StaffDef *part, const std::string &timesig, const std::string &metersig,
    hum::HTp partstart, hum::HTp timetok, hum::HTp metertok)
{
    if ((partstart != NULL) && partstart->isMensLike()) {
        // Don't display time signatures in mensural notation.
        return;
    }

    // Search for a MeterSig child in StaffDef and add one if it does not exist.
    MeterSig *vrvmeter = getMeterSig(part);
    checkMeterSigParameters(vrvmeter, timetok);
    if (!vrvmeter) {
        return;
    }
    if (timetok) {
        setLocationId(vrvmeter, timetok);
    }

    if (metertok) {
        if (*metertok == "*met()") {
            // set time signature to be invisible
            vrvmeter->SetForm(METERFORM_invis);
        }
    }

    // Don't store time signature if there is a mensuration to show
    // (verivio will display both mensuration and time signature.
    bool mensuration = false;
    if (metersig.find("C") != std::string::npos) {
        mensuration = true;
    }
    if (metersig.find("O") != std::string::npos) {
        mensuration = true;
    }

    int top = -1000;
    int bot = -1000;
    int bot2 = -1000;
    if (sscanf(timesig.c_str(), "*M%d/%d%%%d", &top, &bot, &bot2) == 3) {
        // Such as three-triplet whole notes in a 2/1 measure
        if ((metersig == "3") && (bot == 3) && (bot2 == 2)) {
            vrvmeter->SetCount({ { 3 }, MeterCountSign::None });
            vrvmeter->SetUnit(1);
            vrvmeter->SetForm(METERFORM_num);
        }
    }
    else if (sscanf(timesig.c_str(), "*M%d/%d", &top, &bot) == 2) {
        if (bot == 0) {
            if (mensuration) {
                // hide time signature
                vrvmeter->SetForm(METERFORM_invis);
            }
            vrvmeter->SetCount({ { top * 2 }, MeterCountSign::None });
            vrvmeter->SetUnit(1);
        }
        else {
            if (mensuration) {
                // Can't add if there is a mensuration; otherwise,
                // a time signature will be shown.
                vrvmeter->SetForm(METERFORM_invis);
                vrvmeter->SetCount({ { top }, MeterCountSign::None });
                vrvmeter->SetUnit(bot);
            }
            else if (metersig == "3") {
                vrvmeter->SetCount({ { 3 }, MeterCountSign::None });
                vrvmeter->SetUnit(bot);
                vrvmeter->SetForm(METERFORM_num);
            }
            else if (metersig == "2") {
                vrvmeter->SetCount({ { 2 }, MeterCountSign::None });
                vrvmeter->SetUnit(bot);
                vrvmeter->SetForm(METERFORM_num);
            }
            else {
                vrvmeter->SetCount({ { top }, MeterCountSign::None });
                vrvmeter->SetUnit(bot);
            }
        }
    }
    else {
        // some strange time signature which should never occur.
    }
}

//////////////////////////////
//
// HumdrumInput::setTimeSig --
//

template <class ELEMENT>
void HumdrumInput::setTimeSig(ELEMENT element, hum::HTp timesigtok, hum::HTp metersigtok, int staffindex)
{
    if (!timesigtok) {
        // Not allowing meter signatures without a time signature.
        return;
    }

    std::smatch matches;
    std::string metersig;
    if (metersigtok && regex_search(*metersigtok, matches, regex("met\\((.*)\\)"))) {
        metersig = matches[1];
    }

    int count = -1;
    int unit = -1;
    if (timesigtok && regex_search(*timesigtok, matches, regex("^\\*M(\\d+)/(\\d+)%(\\d+)"))) {
        count = stoi(matches[1]);
        // int top = stoi(matches[1]);
        int bot = stoi(matches[2]);
        int bot2 = stoi(matches[3]);
        if ((metersig == "3") && (bot == 3) && (bot2 == 2)) {
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { 3 }, MeterCountSign::None });
            vrvmetersig->SetUnit(1);
            vrvmetersig->SetForm(METERFORM_num);
            checkMeterSigParameters(vrvmetersig, timesigtok);
        }
    }
    else if (timesigtok && regex_search(*timesigtok, matches, regex("^\\*M(\\d+)/(\\d+)"))) {
        count = stoi(matches[1]);
        if (!metersigtok) {
            unit = stoi(matches[2]);
            if (unit == 0) {
                count *= 2;
                unit = 1;
            }
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { count }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
            checkMeterSigParameters(vrvmetersig, timesigtok);
        }
        else if (*metersigtok == "*met()") {
            unit = stoi(matches[2]);
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { std::stoi(matches[1]) }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
            vrvmetersig->SetForm(METERFORM_invis);
        }
        else if (metersig == "3") {
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { 3 }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
            vrvmetersig->SetForm(METERFORM_num);
        }
        else if (metersig == "2") {
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { 2 }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
            vrvmetersig->SetForm(METERFORM_num);
        }
        else if (metersigtok && (metersigtok->find('C') == std::string::npos)
            && (metersigtok->find('O') == std::string::npos)) {
            // Only storing the time signature if there is no mensuration
            // otherwise verovio will display both.
            unit = stoi(matches[2]);
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetCount({ { std::stoi(matches[1]) }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
        }
        else {
            // But always need to provide @meter.unit since timestamps
            // are in reference to it (can't add meter.count since
            // this will also print a time signature.
            // Count now allowed (suppress time signature display with @form="invis").
            unit = stoi(matches[2]);
            if (unit == 0) {
                count *= 2;
                unit = 1;
            }
            MeterSig *vrvmetersig = getMeterSig(element);
            vrvmetersig->SetForm(METERFORM_invis);
            vrvmetersig->SetCount({ { count }, MeterCountSign::None });
            vrvmetersig->SetUnit(unit);
        }
        if (metersigtok) {
            auto ploc = metersigtok->rfind(")");
            if (ploc != std::string::npos) {
                std::string mstring = metersigtok->substr(5, ploc - 5);
                setMeterSymbol(element, mstring, staffindex, NULL, metersigtok);
            }
        }
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if (staffindex < 0) {
        // store time signature change for all staves:
        for (int i = 0; i < (int)ss.size(); ++i) {
            // assuming only a single time sig. at a time.
            ss[i].meter_top = count;
            ss[i].meter_bottom = unit;
        }
    }
    else {
        ss.at(staffindex).meter_top = count;
        ss.at(staffindex).meter_bottom = unit;
    }
}

//////////////////////////////
//
// HumdrumInput::checkMeterSigParameters -- search for enclosure
//     or any other stylings for time signature from attached
//     layout parameters.
//

void HumdrumInput::checkMeterSigParameters(MeterSig *msig, hum::HTp token)
{
    if (!token) {
        return;
    }
    bool parenQ = hasLayoutParameter(token, "TS", "paren");
    bool brackQ = hasLayoutParameter(token, "TS", "brack");
    if (parenQ) {
        msig->SetEnclose(ENCLOSURE_paren);
    }
    else if (brackQ) {
        msig->SetEnclose(ENCLOSURE_brack);
    }
}

//////////////////////////////
//
// HumdrumInput::setKeySig -- Convert a Humdrum keysig to an MEI keysig.
//

template <class ELEMENT>
void HumdrumInput::setKeySig(
    int staffindex, ELEMENT element, const std::string &keysig, hum::HTp keysigtok, hum::HTp keytok, bool secondary)
{
    std::string ks = keysig;
    auto pos = ks.find("]");
    if (pos != std::string::npos) {
        ks = ks.substr(0, pos);
        pos = ks.find("[");
        if (pos != std::string::npos) {
            ks = ks.substr(pos + 1);
        }
    }

    int keynum = -1000;
    if (ks == "") {
        keynum = 0;
    }
    else if (ks == "b-") {
        keynum = -1;
    }
    else if (ks == "f#") {
        keynum = +1;
    }
    else if (ks == "b-e-") {
        keynum = -2;
    }
    else if (ks == "f#c#") {
        keynum = +2;
    }
    else if (ks == "b-e-a-") {
        keynum = -3;
    }
    else if (ks == "f#c#g#") {
        keynum = +3;
    }
    else if (ks == "b-e-a-d-") {
        keynum = -4;
    }
    else if (ks == "f#c#g#d#") {
        keynum = +4;
    }
    else if (ks == "b-e-a-d-g-") {
        keynum = -5;
    }
    else if (ks == "f#c#g#d#a#") {
        keynum = +5;
    }
    else if (ks == "b-e-a-d-g-c-") {
        keynum = -6;
    }
    else if (ks == "f#c#g#d#a#e#") {
        keynum = +6;
    }
    else if (ks == "b-e-a-d-g-c-f-") {
        keynum = -7;
    }
    else if (ks == "f#c#g#d#a#e#b#") {
        keynum = +7;
    }

    int fifthsAdjust = 0;
    if (staffindex >= 0) {
        fifthsAdjust = hum::Convert::base40IntervalToLineOfFifths(m_transpose[staffindex]);
    }
    keynum += fifthsAdjust;

    // Search for a KeySig child in StaffDef and add one if it does not exist.
    KeySig *vrvkeysig = getKeySig(element);
    if (!vrvkeysig) {
        return;
    }
    if (keysigtok) {
        setLocationId(vrvkeysig, keysigtok);
    }

    int keyvalue = keynum;

    if ((keyvalue >= -7) && (keyvalue <= +7)) {
        // standard key signature
        if (keyvalue < 0) {
            vrvkeysig->SetSig(std::make_pair(-keyvalue, ACCIDENTAL_WRITTEN_f));
        }
        else if (keyvalue > 0) {
            vrvkeysig->SetSig(std::make_pair(keyvalue, ACCIDENTAL_WRITTEN_s));
        }
        else if (keyvalue == 0) {
            vrvkeysig->SetSig(std::make_pair(keyvalue, ACCIDENTAL_WRITTEN_NONE));
        }
        else {
            vrvkeysig->SetSig(std::make_pair(-1, ACCIDENTAL_WRITTEN_NONE));
        }
    }
    else {
        // Non-standard keysignature, so give a NONE style (deal with it later).
        prepareNonStandardKeySignature(vrvkeysig, ks, keysigtok);
        return;
    }

    if (secondary && (keyvalue == 0)) {
        // Force cancellation keysignature when there are no
        // sharps/flats in key signature change.
        vrvkeysig->SetSigShowchange(BOOLEAN_true);
    }
    else if (m_show_cautionary_keysig) {
        vrvkeysig->SetSigShowchange(BOOLEAN_true);
    }

    if (!keytok) {
        return;
    }

    // key designation: ^\*([a-gA-G])([#-]*):([a-z]{3})?$
    // pname+mode / accid / mode
    hum::HumRegex hre;
    if (hre.search(*keytok, "^\\*([a-gA-G])([#-]*):([a-z]{3})?$")) {
        std::string letter = hre.getMatch(1);
        std::string accidental = hre.getMatch(2);
        std::string modeabbr = hre.getMatch(3);

        std::string mode;
        if (std::isupper(letter[0])) {
            mode = "major";
        }
        else {
            mode = "minor";
        }
        if (!modeabbr.empty()) {
            if (modeabbr == "dor") {
                mode = "dorian";
            }
            else if (modeabbr == "phr") {
                mode = "phrygian";
            }
            else if (modeabbr == "lyd") {
                mode = "lydian";
            }
            else if (modeabbr == "mix") {
                mode = "mixolydian";
            }
            else if (modeabbr == "aeo") {
                mode = "aeolian";
            }
            else if (modeabbr == "loc") {
                mode = "locrian";
            }
            else if (modeabbr == "ion") {
                mode = "ionian";
            }
        }
        vrvkeysig->SetMode(vrvkeysig->AttKeySigLog::StrToMode(mode));
        switch (std::tolower(letter[0])) {
            case 'c': vrvkeysig->SetPname(PITCHNAME_c); break;
            case 'd': vrvkeysig->SetPname(PITCHNAME_d); break;
            case 'e': vrvkeysig->SetPname(PITCHNAME_e); break;
            case 'f': vrvkeysig->SetPname(PITCHNAME_f); break;
            case 'g': vrvkeysig->SetPname(PITCHNAME_g); break;
            case 'a': vrvkeysig->SetPname(PITCHNAME_a); break;
            case 'b': vrvkeysig->SetPname(PITCHNAME_b); break;
        }
        if (accidental == "-") {
            vrvkeysig->SetAccid(ACCIDENTAL_WRITTEN_f);
        }
        else if (accidental == "#") {
            vrvkeysig->SetAccid(ACCIDENTAL_WRITTEN_s);
        }
    }
}

template <class ELEMENT> void HumdrumInput::setKeySig(ELEMENT *element, hum::HTp keysigtok)
{
    KeySig *vrvkeysig = getKeySig(element);
    if (!vrvkeysig) {
        return;
    }
    if (keysigtok) {
        setLocationId(vrvkeysig, keysigtok);
    }

    std::string plainkey = *keysigtok;
    hum::HumRegex hre;
    hre.replaceDestructive(plainkey, "*k[", "^\\*ok\\[");
    setKeySig(-1, element, plainkey, keysigtok, NULL, false);
}

//////////////////////////////
//
// prepareNonStandardKeySignature --
//

void HumdrumInput::prepareNonStandardKeySignature(KeySig *vrvkeysig, const std::string &ks, hum::HTp keysigtok)
{
    if (!vrvkeysig) {
        return;
    }
    std::vector<std::string> pieces;

    for (int i = 0; i < (int)ks.size(); ++i) {
        if ((ks[i] >= 'a') && (ks[i] <= 'g')) {
            pieces.resize(pieces.size() + 1);
        }
        if (pieces.size() == 0) {
            continue;
        }
        pieces.back() += ks[i];
    }

    for (int i = 0; i < (int)pieces.size(); ++i) {
        if (pieces[i].empty()) {
            // strange error, ignore
            continue;
        }
        KeyAccid *kacc = new KeyAccid();
        vrvkeysig->AddChild(kacc);

        int pclass = pieces[i][0] - 'a';
        switch (pclass) {
            case 0: kacc->SetPname(PITCHNAME_a); break;
            case 1: kacc->SetPname(PITCHNAME_b); break;
            case 2: kacc->SetPname(PITCHNAME_c); break;
            case 3: kacc->SetPname(PITCHNAME_d); break;
            case 4: kacc->SetPname(PITCHNAME_e); break;
            case 5: kacc->SetPname(PITCHNAME_f); break;
            case 6: kacc->SetPname(PITCHNAME_g); break;
        }

        for (int j = 0; j < (int)pieces[i].size(); j++) {
            if (std::isdigit(pieces[i][j])) {
                int octave = pieces[i][j] - '0';
                kacc->SetOct(octave);
                break;
            }
        }

        if (pieces[i].find("##") != std::string::npos) {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_x);
        }
        else if (pieces[i].find("--") != std::string::npos) {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_ff);
        }
        else if (pieces[i].find("#") != std::string::npos) {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_s);
        }
        else if (pieces[i].find("-") != std::string::npos) {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_f);
        }
        else if (pieces[i].find("n") != std::string::npos) {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_n);
        }
        else {
            kacc->SetAccid(ACCIDENTAL_WRITTEN_n);
        }

        if (pieces[i].find("X") != std::string::npos) {
            kacc->SetEnclose(ENCLOSURE_brack);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::setTransposition -- Set the transposition to sounding score.
//

void HumdrumInput::setTransposition(StaffDef *staffDef, const std::string &transpose)
{
    int chromatic = 0;
    int diatonic = 0;
    if (sscanf(transpose.c_str(), "*Trd%dc%dd", &diatonic, &chromatic) != 2) {
        // Transposition is not formatted correctly
        return;
    }
    staffDef->SetTransDiat(-diatonic);
    staffDef->SetTransSemi(-chromatic);
}

//////////////////////////////
//
// HumdrumInput::setDynamicTransposition --
//

void HumdrumInput::setDynamicTransposition(int staffindex, StaffDef *staff, const std::string &itranspose)
{
    int chromatic = 0;
    int diatonic = 0;
    if (sscanf(itranspose.c_str(), "*ITrd%dc%dd", &diatonic, &chromatic) != 2) {
        // Transposition is not formatted correctly
        return;
    }
    staff->SetTransDiat(-diatonic);
    staff->SetTransSemi(-chromatic);

    // Store dynamic transposition to go from sounding score to written:
    m_transpose[staffindex] = hum::Convert::transToBase40(itranspose);
}

//////////////////////////////
//
// HumdrumInput::setClef -- Convert a Humdrum clef to an MEI clef.
//

void HumdrumInput::setClef(StaffDef *staff, const std::string &clef, hum::HTp cleftok, hum::HTp striatok)
{
    // Search for a Clef child in StaffDef and add one if it does not exist.
    Clef *vrvclef = getClef(staff);
    if (!vrvclef) {
        return;
    }
    if (cleftok) {
        setLocationId(vrvclef, cleftok);
    }

    if (clef.find("clefGG") != std::string::npos) {
        vrvclef->SetShape(CLEFSHAPE_GG);
    }
    else if (clef.find("clefG") != std::string::npos) {
        vrvclef->SetShape(CLEFSHAPE_G);
    }
    else if (clef.find("clefF") != std::string::npos) {
        vrvclef->SetShape(CLEFSHAPE_F);
    }
    else if (clef.find("clefC") != std::string::npos) {
        vrvclef->SetShape(CLEFSHAPE_C);
    }
    if (clef.find("clefX") != std::string::npos) {
        vrvclef->SetShape(CLEFSHAPE_perc);
        hum::HumRegex hre;
        int line = -100;
        if (hre.search(clef, "clefX(\\d)")) {
            line = hre.getMatchInt(1);
            if (line > 0) {
                vrvclef->SetLine(line);
            }
        }
        else {
            // Do nothing: it will be centered by verovio
        }
    }

    if (clef.find("yy") != std::string::npos) {
        vrvclef->SetVisible(BOOLEAN_false);
    }

    std::string tok;
    if (cleftok) {
        tok = *cleftok;
    }
    else {
        tok = clef;
    }
    setClefBasicShape(vrvclef, tok);
    setClefStaffLine(vrvclef, tok);
    setClefOctaveDisplacement(vrvclef, tok);
    if (cleftok) {
        checkForClefStyling(vrvclef, cleftok);
    }

    // dummy hierarchy tracking variables:
    std::vector<std::string> elements;
    std::vector<void *> pointers;
    if (cleftok) {
        setClefColorOrEditorial(cleftok, vrvclef, elements, pointers, false);
        setLocationId(vrvclef, cleftok);
    }
}

//////////////////////////////
//
// HumdrumInput::setClefStaffLine -- Set the staff line that the clef
//    is attached.
//    *clefC1 == bottom line of staff
//    *clefC2 == 2nd line from bottom of staff
//    *clefC3 == 3rd line from bottom of staff
//    *clefC4 == 4th line from bottom of staff
//    *clefC5 == 5th line from bottom of staff
//

void HumdrumInput::setClefStaffLine(Clef *clef, const std::string &tok)
{
    if (tok.find("2") != std::string::npos) {
        clef->SetLine(2);
    }
    else if (tok.find("4") != std::string::npos) {
        clef->SetLine(4);
    }
    else if (tok.find("3") != std::string::npos) {
        clef->SetLine(3);
    }
    else if (tok.find("5") != std::string::npos) {
        clef->SetLine(5);
    }
    else if (tok.find("1") != std::string::npos) {
        clef->SetLine(1);
    }
}

//////////////////////////////
//
// HumdrumInput::convertSystemMeasure -- Convert one measure of
//     a Humdrum score into an MEI measure element.
//

bool HumdrumInput::convertSystemMeasure(int &line)
{
    hum::HumdrumFile &infile = m_infiles[0];
    int startline = line;
    int endline = getMeasureEndLine(startline);

    if (endline > infile.getLineCount()) {
        return false;
    }

    if (endline < 0) {
        // empty measure, skip it.  This can happen at the start of
        // a score if there is an invisible measure before the start of the
        // data, or if there is an ending bar before the ending of the data.
        line = -endline;
        return true;
    }
    else if (m_ignore[startline]) {
        // don't convert this measure (!!ignore/!!Xignore toggles)
        line = endline;
        return true;
    }
    else if (m_multirest[line] < 0) {
        // this is a whole-measure rest, but it is part of a multirest sequence.
        line = endline;
        return true;
    }
    else {
        line = endline;
    }

    bool founddatabefore = false;
    for (int q = startline; q >= 0; q--) {
        if (infile[q].isData()) {
            founddatabefore = true;
            break;
        }
    }
    if (!founddatabefore) {
        startline = 0;
    }
    if (infile[startline].isEmpty()) {
        for (int i = startline + 1; i < infile.getLineCount(); ++i) {
            if (infile[i].hasSpines()) {
                startline = i;
                break;
            }
            startline++;
        }
    }

    setupSystemMeasure(startline, endline);

    storeStaffLayerTokensForMeasure(startline, endline);

    auto status = convertMeasureStaves(startline, endline);

    int checkline = startline;
    if (!infile.token(startline, 0)->isBarline()) {
        checkline = getNextBarlineIndex(infile, startline);
    }
    checkForRehearsal(checkline);

    addFTremSlurs();

    storeBeamSpansInStartingMeasure();

    if (m_breaks) {
        checkForLayoutBreak(endline);
    }
    return status;
}

//////////////////////////////
//
// HumdrumInput::getNextBarlineIndex -- Return the next barline row on or after
//     the current index into the file.  If there is none before the first
//     encountered data line, then return the input value.
//

int HumdrumInput::getNextBarlineIndex(hum::HumdrumFile &infile, int startline)
{
    hum::HTp token = infile.token(startline, 0);
    if (token->isBarline()) {
        return startline;
    }
    if (*token == "*-") {
        return startline;
    }
    for (int i = 1; i < infile.getLineCount(); ++i) {
        token = infile.token(startline + i, 0);
        if (token->isBarline()) {
            return startline + i;
        }
        if (token->isData()) {
            return startline;
        }
        if (*token == "*-") {
            return startline + i;
        }
    }
    return startline;
}

//////////////////////////////
//
// HumdrumInput::checkForLayoutBreak --
//

void HumdrumInput::checkForLayoutBreak(int line)
{
    hum::HumdrumFile &infile = m_infiles[0];

    if (line >= infile.getLineCount()) {
        return;
    }
    if (!infile[line].isBarline()) {
        return;
    }
    hum::HTp token = infile.token(line, 0);
    std::string group;

    group = token->getLayoutParameter("LB", "g");
    if (!group.empty()) {
        std::string tstring = removeCommas(group);
        Sb *sb = new Sb;
        m_layoutInformation = LAYOUT_ENCODED;
        if (m_currentending) {
            m_currentending->AddChild(sb);
        }
        else {
            m_sections.back()->AddChild(sb);
        }
        setLocationId(sb, token);
        appendTypeTag(sb, tstring);
        return;
    }

    group = token->getLayoutParameter("PB", "g");
    if (!group.empty()) {
        std::string tstring = removeCommas(group);
        Pb *pb = new Pb();
        // Sb *pb = new Sb();  // suppress page break encodings
        m_layoutInformation = LAYOUT_ENCODED;
        if (m_currentending) {
            m_currentending->AddChild(pb);
        }
        else {
            m_sections.back()->AddChild(pb);
        }
        setLocationId(pb, token);
        appendTypeTag(pb, tstring);
        return;
    }
}

//////////////////////////////
//
// HumdrumInput::removeCommas --
//

std::string HumdrumInput::removeCommas(const std::string &input)
{
    std::string output = input;
    for (int i = 0; i < (int)output.size(); ++i) {
        if (output[i] == ',') {
            output[i] = ' ';
        }
        else if (output[i] == ';') {
            output[i] = ' ';
        }
        else if (output[i] == ':') {
            output[i] = ' ';
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::checkForOmd --
//

void HumdrumInput::checkForOmd(int startline, int endline)
{
    if (m_mens) {
        // need to avoid rhythm parsing, so ignore not for mensural music
        return;
    }
    hum::HumdrumFile &infile = m_infiles[0];
    if (m_omd > infile[startline].getDurationFromStart()) {
        return;
    }

    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    if (staffstarts.size() == 0) {
        return;
    }
    std::string key;
    std::string value;
    int index = -1;
    for (int i = startline; i <= endline; ++i) {
        if (infile[i].isData()) {
            break;
        }
        if (infile[i].isBarline()) {
            hum::HumRegex hre;
            hum::HTp token = infile[i].token(0);
            int number = -1;
            if (hre.search(token, "=(\\d+)")) {
                number = hre.getMatchInt(1);
            }
            if ((!value.empty()) && (number > 1)) {
                // don't print initial OMD if a musical excerpt.
                return;
            }
        }
        if (!infile[i].isReference()) {
            continue;
        }
        key = infile[i].getReferenceKey();
        if (key == "OMD") {
            index = i;
            value = infile[i].getReferenceValue();
            // Don't break: search for the last OMD in a non-data region.
        }
    }

    if (!value.empty()) {
        Tempo *tempo = new Tempo();
        hum::HTp token = infile.token(index, 0);
        hum::HumNum timepos = token->getDurationFromStart();
        if (timepos > 0) {
            double midibpm = getMmTempo(token);
            if (midibpm > 0) {
                tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
                m_midibpm = midibpm;
            }
            else {
                // check for *MM marker before OMD
                midibpm = getMmTempoForward(token);
                if (midibpm > 0) {
                    tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
                    m_midibpm = midibpm;
                }
            }
        }
        if (index >= 0) {
            setLocationId(tempo, token);
        }
        addChildBackMeasureOrSection(tempo);
        setTempoContent(tempo, value);
        tempo->SetTstamp(1.0);
        setStaff(tempo, 1);
        m_omd = infile[index].getDurationFromStart();
    }
}

//////////////////////////////
//
// HumdrumInput::addChildBackMeasureOrSection -- Add to the current measure, or add to section
//     if there is not measure.
//

template <class ELEMENT> void HumdrumInput::addChildBackMeasureOrSection(ELEMENT element)
{
    if (m_measure) {
        m_measure->AddChildBack(element);
    }
    else {
        m_sections.back()->AddChild(element);
    }
}

//////////////////////////////
//
// HumdrumInput::setStaff -- Set the staff attribute to given integer.
//

template <class ELEMENT> void HumdrumInput::setStaff(ELEMENT element, int staffnum)
{
    xsdPositiveInteger_List stafflist;
    stafflist.push_back(staffnum);
    element->SetStaff(stafflist);
}

//////////////////////////////
//
// HumdrumInput::setStaffBetween -- Set to the given staff and the next staff below.
//

template <class ELEMENT> void HumdrumInput::setStaffBetween(ELEMENT element, int staffnum)
{
    xsdPositiveInteger_List stafflist;
    stafflist.push_back(staffnum);
    stafflist.push_back(staffnum + 1);
    element->SetStaff(stafflist);
}

//////////////////////////////
//
// HumdrumInput::setN --
//

template <class ELEMENT> void HumdrumInput::setN(ELEMENT element, int nvalue, hum::HTp tok)
{
    std::string name = element->GetClassName();
    if (tok && (name == "Ending")) {
        // Check if there is a LO:TX text to replace number.
        std::string textlabel = tok->getLayoutParameter("TX", "t");
        if (!textlabel.empty()) {
            element->SetN(textlabel);
        }
        else {
            element->SetN(to_string(nvalue));
        }
    }
    else {
        element->SetN(to_string(nvalue));
    }
}

//////////////////////////////
//
// HumdrumInput::storeStaffLayerTokensForMeasure -- Store lists of notation
//   data by staff and layer.
//

void HumdrumInput::storeStaffLayerTokensForMeasure(int startline, int endline)
{
    hum::HumdrumFile &infile = m_infiles[0];
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    const std::vector<int> &rkern = m_rkern;
    std::vector<std::vector<std::vector<hum::HTp>>> &lt = m_layertokens;

    lt.clear();
    lt.resize(staffstarts.size());

    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        lt[i].clear();
    }

    int lasttrack = -1;
    int track = -1;
    int staffindex = -1;
    int layerindex = 0;

    // First need to pre-allocate layer information so that clefs can
    // be inserted into partial layers (which otherwise may not have
    // been created before the clef needs to be inserted).

    for (int i = startline; i <= endline; ++i) {
        if ((i > startline) || (i < endline)) {
            if (infile[i].isData() && infile[i - 1].isData()) {
                // spining cannot change between data lines
                // so do not bother to check.
                continue;
            }
        }
        if (!infile[i].hasSpines()) {
            continue;
        }
        // check for the maximum size of each spine (check staff
        // for maximum layer count):
        lasttrack = -1;
        for (int j = 0; j < infile[i].getFieldCount(); j++) {
            hum::HTp token = infile[i].token(j);
            if (!token->isStaff()) {
                continue;
            }
            if (token->isDataType("**kernyy")) {
                continue;
            }
            track = token->getTrack();
            if (track != lasttrack) {
                layerindex = 0;
            }
            else {
                layerindex++;
            }
            if (track != lasttrack) {
                lasttrack = track;
                continue;
            }
            staffindex = rkern[track];
            if (staffindex < 0) {
                cerr << "STAFF INDEX PROBLEM FOR TRACK " << track << endl;
            }
            if ((int)lt[staffindex].size() < layerindex + 1) {
                lt[staffindex].resize(lt[staffindex].size() + 1);
            }
        }
    }

    for (int i = startline; i <= endline; ++i) {
        if (!infile[i].hasSpines()) {
            continue;
        }
        lasttrack = -1;
        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile[i].token(j);
            track = token->getTrack();
            if (track < 1) {
                continue;
            }
            staffindex = rkern[track];
            if (staffindex < 0) {
                continue;
            }
            if (track != lasttrack) {
                layerindex = 0;
            }
            else if (!token->isPrimaryStrophe()) {
                // Do not increment layer index for
                // secondary strophes.
                // Also ignore non-primary strophes for now.
                continue;
            }
            else {
                layerindex++;
            }
            lasttrack = track;
            if (token->isData() && token->isNull()) {
                // keeping null interpretations to search for clef
                // in primary layer for secondary layer duplication.
                if (token->getLinkedParameterSetCount() == 0) {
                    continue;
                }
            }
            if (token->isCommentLocal() && token->isNull()) {
                // don't store empty comments as well. (maybe ignore all
                // comments anyway).
                continue;
            }
            if ((int)lt[staffindex].size() < layerindex + 1) {
                lt[staffindex].resize(lt[staffindex].size() + 1);
                lt[staffindex].back().clear(); // probably not necessary
            }

            if (token->isBarline() && !token->allSameBarlineStyle()) {
                if (infile[i].hasDataStraddle()) {
                    if (token->find('-') != std::string::npos) {
                        // do not store partial invisible barlines
                        continue;
                    }
                }
            }
            lt[staffindex][layerindex].push_back(token);

            if ((layerindex == 0) && (token->isClef())) {

                int layercount = getCurrentLayerCount(token);

                // Duplicate clef in all layers (needed for cases when
                // a secondary layer ends before the end of a measure.

                for (int k = layercount; k < (int)lt[staffindex].size(); k++) {
                    lt[staffindex][k].push_back(token);
                }
            }

            // duplicate *join to all secondary layers
            if ((layerindex == 0) && (*token == "*join")) {
                for (int k = 1; k < (int)lt[staffindex].size(); k++) {
                    lt[staffindex][k].push_back(token);
                }
            }

            // duplicate *Xjoin to all secondary layers
            if ((layerindex == 0) && (*token == "*Xjoin")) {
                for (int k = 1; k < (int)lt[staffindex].size(); k++) {
                    lt[staffindex][k].push_back(token);
                }
            }
        }
    }

    if (m_debug) {
        printMeasureTokens();
    }
}

//////////////////////////////
//
// HumdrumInput::getCurrentLayerCount -- Given a token in layer 1
//    of a staff, count how many active layers there are at the
//    same time.
//

int HumdrumInput::getCurrentLayerCount(hum::HTp token)
{
    int output = 1;
    int ttrack = token->getTrack();
    hum::HTp current = token->getNextFieldToken();
    while (current) {
        int track = current->getTrack();
        if (track != ttrack) {
            break;
        }
        output++;
        current = current->getNextFieldToken();
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::convertMeasureStaves -- fill in a measure with the
//    individual staff elements for each part.
//

bool HumdrumInput::convertMeasureStaves(int startline, int endline)
{
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;

    std::vector<int> layers = getStaffLayerCounts();

    if (m_fb) {
        // This function needs to come before notes so that
        // the *above/*below markers can be used to set
        // the location of tuplets within each measure.
        // But placing here will cause the figured bass
        // elements to be added before <staff> in <measure>
        // which is not the typical placement ordering.
        addFiguredBassForMeasure(startline, endline);
    }

    // pre-allocate
    std::vector<Staff *> stafflist(staffstarts.size());
    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        stafflist[i] = new Staff();
        setLocationId(stafflist[i], staffstarts[i]);
        addChildMeasureOrSection(stafflist[i]);
    }

    checkForOmd(startline, endline);

    bool status = true;
    for (int i = 0; i < (int)staffstarts.size(); ++i) {
        m_currentstaff = i + 1;
        m_staff = stafflist[i];
        m_staff->SetN(m_currentstaff);
        status &= convertMeasureStaff(staffstarts[i]->getTrack(), startline, endline, i + 1, layers[i]);
        if (!status) {
            break;
        }
    }

    if (m_harm || m_degree) {
        addHarmFloatsForMeasure(startline, endline);
    }

    if (m_fing) {
        addFingeringsForMeasure(startline, endline);
    }

    if (m_string) {
        addStringNumbersForMeasure(startline, endline);
    }

    return status;
}

//////////////////////////////
//
// checkForLineContinuations -- Temporary code until fb line
//      extensions are implemented.  If the previous **fb token
//      contains a line extension for this null token, then add
//      underscores.
//

void HumdrumInput::checkForLineContinuations(hum::HTp token)
{
    if (!token->isNull()) {
        return;
    }
    hum::HTp resolved = token->resolveNull();
    if (resolved->find("_") == std::string::npos) {
        return;
    }

    int spinetrack = token->getTrack();
    Harm *harm = new Harm();
    Fb *fb = new Fb();

    if (token->isDataType("**fba")) {
        if (m_placement[spinetrack] == 0) {
            setPlaceRelStaff(harm, "above", false);
        }
        else if (m_placement[spinetrack] == -1) {
            setPlaceRelStaff(harm, "below", false);
        }
        else if (m_placement[spinetrack] == +1) {
            setPlaceRelStaff(harm, "above", false);
        }
    }
    else {
        if (m_placement[spinetrack] == -1) {
            setPlaceRelStaff(harm, "below", false);
        }
        else if (m_placement[spinetrack] == +1) {
            setPlaceRelStaff(harm, "above", false);
        }
    }
    harm->AddChild(fb);

    hum::HumRegex hre;
    std::vector<std::string> pieces;
    hre.split(pieces, *resolved, " ");
    for (int i = 0; i < (int)pieces.size(); ++i) {
        if (pieces[i].find("_") != std::string::npos) {
            F *f = new F();
            Text *text = new Text();
            std::u32string wtext = U"_";
            text->SetText(wtext);
            f->AddChild(text);
            fb->AddChild(f);
        }
        else {
            // place holder
            F *f = new F();
            fb->AddChild(f);
        }
    }

    int kerntrack = -1;
    hum::HTp current = token;
    while (current) {
        if (current->isKernLike()) {
            kerntrack = current->getTrack();
        }
        current = current->getPreviousFieldToken();
    }

    addChildMeasureOrSection(harm);
    int staffindex = 0;
    if (kerntrack >= 0) {
        staffindex = m_rkern[kerntrack];
    }
    hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    harm->SetTstamp(tstamp.getFloat());
    setStaff(harm, staffindex + 1);
    setLocationId(harm, token);
    setLocationId(fb, token);
}

//////////////////////////////
//
// HumdrumInput::addChildMeasureOrSection -- Add element to measure if exists; otherwise, add to section.
//

template <class ELEMENT> void HumdrumInput::addChildMeasureOrSection(ELEMENT element, Measure *measure)
{
    if (measure) {
        measure->AddChild(element);
    }
    else if (m_measure) {
        m_measure->AddChild(element);
    }
    else {
        m_sections.back()->AddChild(element);
    }
}

//////////////////////////////
//
// HumdrumInput::addFiguredBassForMeasure --
//

void HumdrumInput::addFiguredBassForMeasure(int startline, int endline)
{
    if (!m_measure) {
        return;
    }
    hum::HumdrumFile &infile = m_infiles[0];

    for (int i = startline; i < endline; ++i) {
        if (infile[i].isInterpretation()) {
            int staffindex = 0;
            for (int j = 0; j < infile[i].getFieldCount(); ++j) {
                if (j >= 999) {
                    break;
                }
                hum::HTp token = infile.token(i, j);
                if (token->isKernLike()) {
                    staffindex++;
                }
                else if (token->isMensLike()) {
                    staffindex++;
                }
                if (!(token->isDataType("**fb") || token->isDataType("**fba") || token->isDataType("**Bnum"))) {
                    continue;
                }
                int track = token->getTrack();
                if (token->compare("*above") == 0) {
                    m_placement[track] = +1;
                    int index = staffindex - 1;
                    index = index < 0 ? 0 : index;
                    m_fbstates.at(index) = +1;
                }
                else if (token->compare("*below") == 0) {
                    m_placement[track] = -1;
                    int index = staffindex - 1;
                    index = index < 0 ? 0 : index;
                    m_fbstates.at(index) = -1;
                }
                else if (token->compare("*auto") == 0) {
                    m_placement[track] = 0;
                    int index = staffindex - 1;
                    index = index < 0 ? 0 : index;
                    m_fbstates.at(index) = 0;
                }
                else if (token->compare("*reverse") == 0) {
                    m_reverse[track] = +1;
                }
                else if (token->compare("*Xreverse") == 0) {
                    m_reverse[track] = 0;
                }
                else if (token->compare("*absolute") == 0) {
                    m_absolute[track] = +1;
                }
                else if (token->compare("*Xabsolute") == 0) {
                    m_absolute[track] = 0;
                }
                else if (token->compare("*slash") == 0) {
                    m_slash[track] = +1;
                }
                else if (token->compare("*Xslash") == 0) {
                    m_slash[track] = 0;
                }
            }
            continue;
        }

        if (!infile[i].isData()) {
            continue;
        }
        int kerntrack = 0;
        int spinetrack = 0;
        int active = true;

        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile.token(i, j);
            std::string exinterp = token->getDataType();
            if ((exinterp != "**kern") && (exinterp.find("kern") != std::string::npos)) {
                active = false;
            }
            if (token->isDataType("**kern")) {
                kerntrack = token->getTrack();
                active = true;
            }
            if (!active) {
                continue;
            }
            if (!(token->isDataType("**fb") || token->isDataType("**fba") || token->isDataType("**Bnum"))) {
                continue;
            }
            if (token->isNull()) {
                checkForLineContinuations(token);
                continue;
            }
            if (token->compare("_") == 0) {
                continue;
            }
            spinetrack = token->getTrack();
            Harm *harm = new Harm();
            Fb *fb = new Fb();

            if (token->isDataType("**fba")) {
                if (m_placement[spinetrack] == 0) {
                    setPlaceRelStaff(harm, "above", false);
                }
                else if (m_placement[spinetrack] == -1) {
                    setPlaceRelStaff(harm, "below", false);
                }
                else if (m_placement[spinetrack] == +1) {
                    setPlaceRelStaff(harm, "above", false);
                }
            }
            else {
                if (m_placement[spinetrack] == -1) {
                    setPlaceRelStaff(harm, "below", false);
                }
                else if (m_placement[spinetrack] == +1) {
                    setPlaceRelStaff(harm, "above", false);
                }
            }
            harm->AddChild(fb);

            std::vector<std::string> pieces = splitFBString(*token, " ");
            std::vector<std::u32string> content = cleanFBString(pieces, token);
            if (content.empty()) {
                // do not include an empty fb see issue #1096
                continue;
            }

            for (int k = 0; k < (int)content.size(); ++k) {
                F *f = new F();
                if ((pieces[k] == "x") || (pieces[k] == "X")) {
                    // suppress contents of <f>
                }
                else {
                    Text *text = new Text();
                    auto pos = pieces[k].find(":");
                    if (pos != std::string::npos) {
                        std::vector<std::string> subpieces = splitFBString(pieces[k], ":");
                        std::u32string newtext = cleanFBString2(subpieces, token);
                        text->SetText(newtext);
                    }
                    else {
                        text->SetText(content[k]);
                    }
                    f->AddChild(text);
                }
                fb->AddChild(f);
                if (pieces[k].find("_") != std::string::npos) {
                    // Does not display anything at the moment
                    // so a "_" character was added to content[k].
                    // This can be removed when extenders are
                    // implemented.
                    // f->SetExtender(BOOLEAN_true);
                }
                if (content.size() == 1) {
                    setLocationId(f, token);
                }
                else {
                    setLocationIdNSuffix(f, token, k + 1);
                }
            }

            addChildMeasureOrSection(harm);
            int staffindex = m_rkern[kerntrack];
            if (m_placement.at(spinetrack)) {
                m_fbstates.at(staffindex) = m_placement.at(spinetrack);
            }
            hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
            harm->SetTstamp(tstamp.getFloat());
            setStaff(harm, staffindex + 1);
            setLocationId(harm, token);
            setLocationId(fb, token);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::addFingeringsForMeasure --
//

void HumdrumInput::addFingeringsForMeasure(int startline, int endline)
{
    if (!m_measure) {
        return;
    }
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    hum::HumdrumFile &infile = m_infiles[0];
    bool aboveQ = true;
    hum::HumRegex hre;
    std::vector<std::string> nums;

    for (int i = startline; i < endline; ++i) {
        if (!infile[i].isData()) {
            continue;
        }
        int track = 0;
        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile.token(i, j);
            if (token->isDataType("**kern")) {
                track = token->getTrack();
            }
            if (token->isNull()) {
                continue;
            }
            if (!token->isDataType("**fing")) {
                continue;
            }
            std::string content = token->getText();
            if (content.empty()) {
                continue;
            }

            hre.split(nums, content, " ");

            int ztrack = token->getTrack();
            aboveQ = true;
            if ((j > 0) && infile.token(i, j - 1)->isDataType("**fing")) {
                int ptrack = infile.token(i, j - 1)->getTrack();
                if (ztrack == ptrack) {
                    aboveQ = false;
                }
            }

            int staffindex = m_rkern[track];
            int maxstaff = (int)staffstarts.size();

            if (token->getValue("auto", "place") == "above") {
                aboveQ = true;
            }
            else if (token->getValue("auto", "place") == "below") {
                aboveQ = false;
            }

            if (aboveQ) {
                for (int k = 0; k < (int)nums.size(); k++) {
                    insertFingerNumberInMeasure(nums[k], staffindex, token, maxstaff, aboveQ);
                }
            }
            else {
                for (int k = 0; k < (int)nums.size(); k++) {
                    insertFingerNumberInMeasure(nums[k], staffindex, token, maxstaff, aboveQ);
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::insertFingerNumberInMeasure --
//

void HumdrumInput::insertFingerNumberInMeasure(
    const std::string &text, int staffindex, hum::HTp token, int maxstaff, bool aboveQ)
{

    Dir *dir = new Dir();
    int xstaffindex = 0;
    if (staffindex >= 0) {
        xstaffindex = staffindex;
        setStaff(dir, staffindex + 1);
    }
    else {
        // data is not attached to a **kern spine since it comes before
        // any **kern data.  Treat it as attached to the bottom staff.
        // (or the top staff depending on @place="above|below".
        xstaffindex = maxstaff - 1;
        setStaff(dir, xstaffindex + 1);
    }

    Rend *rend = new Rend();
    data_FONTSIZE fs;
    fs.SetTerm(FONTSIZETERM_x_small);
    rend->SetFontsize(fs);
    rend->SetFontstyle(FONTSTYLE_normal);
    addTextElement(rend, text);
    dir->AddChild(rend);
    appendTypeTag(dir, "fingering");
    if (aboveQ) {
        setPlaceRelStaff(dir, "above", false);
    }
    else {
        setPlaceRelStaff(dir, "below", false);
    }
    addChildMeasureOrSection(dir);
    setLocationId(dir, token);

    // Previously used @tstamp, now use @startid of note/chord;
    // hum::HumNum tstamp = getMeasureTstamp(token, xstaffindex);
    // dir->SetTstamp(tstamp.getFloat());
    linkFingeringToNote(dir, token, xstaffindex);
}

//////////////////////////////
//
// HumdrumInput::linkFingeringToNote -- link to the note/chord in the highest layer
//    (left-most spine) that is not a null data token. If no notes/chords at
//    the position of the finger (such as perhaps due to a finger-change, then
//    use a @tstamp rather than a @startid to place the fingering.
//

void HumdrumInput::linkFingeringToNote(Dir *dir, hum::HTp token, int xstaffindex)
{
    // token should be a **fing, so search for the **kern that it
    // matches to the left, and then search for the last non-null
    // kern token to link to the fingering (figure out later how to link
    // to notes in secondary layers which will probably be done by
    // placing fingerings in subspines of the **fing spine.

    hum::HTp linkednote = NULL;
    int linktrack = -1;

    hum::HumdrumLine &line = *token->getLine();
    int startfield = token->getFieldIndex();

    for (int i = startfield - 1; i >= 0; i--) {
        hum::HTp testtok = line.token(i);
        if (!testtok->isKernLike()) {
            continue;
        }
        linktrack = testtok->getTrack();
        for (int j = i; j >= 0; j--) {
            testtok = line.token(j);
            int ttrack = testtok->getTrack();
            if (ttrack != linktrack) {
                break;
            }
            if (!testtok->isNull()) {
                linkednote = testtok;
            }
        }
        break;
    }
    if (!linkednote) {
        // use a timestamp to place the fingering
        hum::HumNum tstamp = getMeasureTstamp(token, xstaffindex);
        dir->SetTstamp(tstamp.getFloat());
    }
    else {
        std::string startid;
        if (linkednote->find(" ") != std::string::npos) {
            startid = getLocationId("chord", linkednote);
        }
        else {
            startid = getLocationId("note", linkednote);
        }
        dir->SetStartid("#" + startid);
    }
}

//////////////////////////////
//
// HumdrumInput::addStringNumbersForMeasure --
//

void HumdrumInput::addStringNumbersForMeasure(int startline, int endline)
{
    if (!m_measure) {
        return;
    }
    int xstaffindex;
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    hum::HumdrumFile &infile = m_infiles[0];
    for (int i = startline; i < endline; ++i) {
        if (!infile[i].isData()) {
            continue;
        }
        int track = 0;
        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile.token(i, j);
            if (token->isDataType("**kern")) {
                track = token->getTrack();
            }
            if (token->isNull()) {
                continue;
            }
            if (!token->isDataType("**string")) {
                continue;
            }
            Harm *harm = new Harm();
            Text *text = new Text();

            int staffindex = m_rkern[track];

            if (staffindex >= 0) {
                xstaffindex = staffindex;
                setStaff(harm, staffindex + 1);
            }
            else {
                // data is not attached to a **kern spine since it comes before
                // any **kern data.  Treat it as attached to the bottom staff.
                // (or the top staff depending on @place="above|below".
                xstaffindex = (int)staffstarts.size() - 1;
                setStaff(harm, xstaffindex + 1);
            }
            std::u32string content;
            content = cleanStringString(*token);
            text->SetText(content);
            harm->AddChild(text);
            addChildMeasureOrSection(harm);
            hum::HumNum tstamp = getMeasureTstamp(token, xstaffindex);
            harm->SetTstamp(tstamp.getFloat());
            appendTypeTag(harm, "string");
            setLocationId(harm, token);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::addHarmFloatsForMeasure -- Does not handle chords in **deg yet.
//

void HumdrumInput::addHarmFloatsForMeasure(int startline, int endline)
{
    if (!m_measure) {
        return;
    }
    hum::HumRegex hre;
    int xstaffindex;
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    hum::HumdrumFile &infile = m_infiles[0];
    for (int i = startline; i < endline; ++i) {
        if (infile[i].isInterpretation()) {
            for (int j = 0; j < infile[i].getFieldCount(); j++) {
                if (hre.search(infile.token(i, j), "^\\*color:(.*)")) {
                    int ctrack = infile.token(i, j)->getTrack();
                    int strack = infile.token(i, j)->getSubtrack();
                    m_spine_color[ctrack][strack] = hre.getMatch(1);
                    if (strack == 1) {
                        m_spine_color[ctrack][0] = m_spine_color[ctrack][1];
                    }
                    else if (strack == 0) {
                        for (int z = 1; z < (int)m_spine_color[ctrack].size(); z++) {
                            m_spine_color[ctrack][z] = m_spine_color[ctrack][0];
                        }
                    }
                }
            }
        }
        if (!infile[i].isData()) {
            continue;
        }
        int track = 0;
        int active = true;
        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile.token(i, j);
            std::string exinterp = token->getDataType();
            if ((exinterp != "**kern") && (exinterp.find("kern") != std::string::npos)) {
                active = false;
                continue;
            }
            if (token->isDataType("**kern")) {
                track = token->getTrack();
                active = true;
            }
            if (token->getValueInt("auto", "hidden")) {
                continue;
            }
            if (!active) {
                continue;
            }
            if (token->isNull()) {
                continue;
            }
            bool isCData = token->getDataType().compare(0, 7, "**cdata") == 0;
            bool isDegree = (token->getDataType() == "**deg") || (token->getDataType() == "**degree");
            if (!(token->isDataType("**mxhm") || token->isDataType("**harm") || token->isDataType("**rhrm") || isCData
                    || isDegree)) {
                continue;
            }
            if (token->find("yy") != std::string::npos) {
                // skip displaying the scale degree
                continue;
            }
            if (isDegree && (token->find('r') != std::string::npos)) {
                if (token->find('0') == std::string::npos) {
                    // Don't add rest marker data (otherwise it will be labeled as the "0" scale degree).
                    continue;
                }
            }
            Harm *harm = new Harm();
            Text *text = new Text();
            Rend *rend = NULL;

            addChildMeasureOrSection(harm);

            int line = token->getLineIndex();
            int field = token->getFieldIndex();

            std::string ccolor = getSpineColor(line, field);
            if (!ccolor.empty()) {
                rend = new Rend();
                rend->SetColor(ccolor);
                harm->AddChild(rend);
                rend->AddChild(text);
                if (token->getValueInt("auto", "circle")) {
                    rend->SetRend(TEXTRENDITION_circle);
                }
                else if (token->getValueInt("auto", "box")) {
                    rend->SetRend(TEXTRENDITION_box);
                }
            }
            else {
                if (token->getValueInt("auto", "circle")) {
                    rend = new Rend();
                    rend->SetRend(TEXTRENDITION_circle);
                    harm->AddChild(rend);
                    rend->AddChild(text);
                }
                else if (token->getValueInt("auto", "box")) {
                    rend = new Rend();
                    rend->SetRend(TEXTRENDITION_box);
                    harm->AddChild(rend);
                    rend->AddChild(text);
                }
                else {
                    if (token->getValueInt("auto", "circle")) {
                        rend = new Rend();
                        rend->SetRend(TEXTRENDITION_circle);
                        harm->AddChild(rend);
                        rend->AddChild(rend);
                    }
                    else if (token->getValueInt("auto", "box")) {
                        rend = new Rend();
                        rend->SetRend(TEXTRENDITION_box);
                        harm->AddChild(rend);
                        rend->AddChild(rend);
                    }
                    else {
                        harm->AddChild(text);
                    }
                }
            }

            std::string octave;
            if (token->isDataType("**degree")) {
                int octaveQ = !token->getValueInt("auto", "Xoctave");
                if (octaveQ) {
                    hum::HumRegex hre2;
                    if (hre2.search(token, "/(\\d+)")) {
                        octave = hre2.getMatch(1);
                    }
                }
            }

            if (!octave.empty()) {
                Rend *subrend = new Rend();
                Text *subtext = new Text();
                subrend->AddChild(subtext);
                subrend->SetRend(TEXTRENDITION_sub);
                subrend->SetType("octave");
                std::u32string subcontent = UTF8to32(octave);
                subtext->SetText(subcontent);
                if (rend) {
                    rend->AddChild(subrend);
                }
                else {
                    harm->AddChild(subrend);
                }
            }

            std::string tracktext = getTrackText(token);
            harm->SetN(tracktext);

            int staffindex = m_rkern[track];

            if (staffindex >= 0) {
                xstaffindex = staffindex;
                setStaff(harm, staffindex + 1);
            }
            else {
                // data is not attached to a **kern spine since it comes before
                // any **kern data.  Treat it as attached to the bottom staff.
                // (or the top staff depending on @place="above|below".
                xstaffindex = (int)staffstarts.size() - 1;
                setStaff(harm, xstaffindex + 1);
            }

            std::string datatype = token->getDataType();
            if (datatype.compare(0, 8, "**cdata-") == 0) {
                std::string subdatatype = datatype.substr(8);
                if (!subdatatype.empty()) {
                    appendTypeTag(harm, subdatatype);
                }
            }
            else if (datatype.compare(0, 5, "**deg") == 0) {
                std::string subdatatype = datatype.substr(2);
                if (!subdatatype.empty()) {
                    appendTypeTag(harm, subdatatype);
                }
            }

            std::u32string content;
            std::u32string precontent;
            std::u32string postcontent;
            bool preleapQ = false;
            bool preupdirQ = false;
            bool predowndirQ = false;
            bool postleapQ = false;
            bool postupdirQ = false;
            bool postdowndirQ = false;
            if (datatype == "**harm") {
                content = cleanHarmString2(*token);
            }
            else if (datatype == "**rhrm") {
                content = cleanHarmString3(*token);
            }
            else if (isDegree) {
                content = cleanDegreeString(token);

                // *dir/*Xdir: do/do_not show melodic approaches (directions)
                int dirQ = !token->getValueInt("auto", "Xdir");

                if (dirQ) {
                    // note: token is presumend to not be a chord

                    // process melodic departure information
                    int upcount = 0;
                    int downcount = 0;
                    std::string firstnote;
                    for (int m = 0; m < (int)token->size(); m++) {
                        if (token->at(m) == ' ') {
                            // currently only processing first note of chords
                            break;
                        }
                        else {
                            firstnote.push_back(token->at(m));
                        }
                        if (token->at(m) == '^') {
                            upcount++;
                        }
                        else if (token->at(m) == 'v') {
                            downcount++;
                        }
                        if ((m > 0) && (token->at(m) == 'y') && (token->at(m - 1) == '^')) {
                            upcount = 0;
                        }
                        if ((m > 0) && (token->at(m) == 'y') && (token->at(m - 1) == 'v')) {
                            downcount = 0;
                        }
                    }

                    if (upcount == 1) {
                        precontent = U"\u2197"; // single up diagonal arrow
                        // precontent = U"\u2191"; // up arrow
                        preupdirQ = true;
                    }
                    else if (upcount >= 2) {
                        // precontent = U"\u21d7"; // double up diagonal arrow
                        precontent = U"\u2b08"; // thick up diagonal arrow
                        preupdirQ = true;
                        preleapQ = true;
                    }
                    else if (downcount == 1) {
                        precontent = U"\u2198"; // single up diagonal arrow
                        // precontent = U"\u2193"; // down arrow
                        predowndirQ = true;
                    }
                    else if (downcount >= 2) {
                        // precontent = U"\u21d8"; // double down arrow
                        precontent = U"\u2b0a"; // thick down diagonal arrow
                        predowndirQ = true;
                        preleapQ = true;
                    }
                }

                if (dirQ) {
                    // note: token is presumend to not be a chord.

                    // process melodic departure information
                    int upcount = 0;
                    int downcount = 0;
                    for (int m = 0; m < (int)token->size(); m++) {
                        if (token->at(m) == ' ') {
                            // only process first note of chord for now
                            break;
                        }
                        if (token->at(m) == '\'') {
                            upcount++;
                        }
                        else if (token->at(m) == ',') {
                            downcount++;
                        }
                        if ((m > 0) && (token->at(m) == 'y') && (token->at(m - 1) == '\'')) {
                            upcount = 0;
                        }
                        if ((m > 0) && (token->at(m) == 'y') && (token->at(m - 1) == ',')) {
                            downcount = 0;
                        }
                    }
                    if (upcount == 1) {
                        postcontent = U"\u2197"; // single up diagonal arrow
                        // postcontent = U"\u2191"; // up arrow
                        postupdirQ = true;
                    }
                    else if (upcount >= 2) {
                        // postcontent = U"\u21d7"; // double up diagonal arrow
                        postcontent = U"\u2b08"; // thick up diagonal arrow
                        postupdirQ = true;
                        postleapQ = true;
                    }
                    else if (downcount == 1) {
                        postcontent = U"\u2198"; // single up diagonal arrow
                        // postcontent = U"\u2193"; // down arrow
                        postdowndirQ = true;
                    }
                    else if (downcount >= 2) {
                        // postcontent = U"\u21d8"; // double down arrow
                        postcontent = U"\u2b0a"; // thick down diagonal arrow
                        postdowndirQ = true;
                        postleapQ = true;
                    }
                }
            }

            else if (isCData) {
                content = UTF8to32(*token);
            }
            else {
                content = cleanHarmString(*token);
            }

            text->SetText(content);

            if (isDegree && (preupdirQ || predowndirQ) && !precontent.empty()) {
                Rend *prerend = new Rend();
                Text *pretext = new Text();
                prerend->AddChild(pretext);
                pretext->SetText(precontent);
                if (preupdirQ) {
                    prerend->SetRend(TEXTRENDITION_sub);
                    if (preleapQ) {
                        setFontsize(prerend, "", "120%");
                        prerend->SetType("approach-up-leap");
                    }
                    else {
                        prerend->SetType("approach-up-step");
                    }
                }
                else if (predowndirQ) {
                    prerend->SetRend(TEXTRENDITION_sup);
                    if (preleapQ) {
                        setFontsize(prerend, "", "120%");
                        prerend->SetType("approach-down-leap");
                    }
                    else {
                        prerend->SetType("approach-down-step");
                    }
                }
                if (rend) {
                    rend->InsertChild(prerend, 0);
                    prerend->SetParent(rend);
                }
                else {
                    harm->InsertChild(prerend, 0);
                    prerend->SetParent(harm);
                }
            }

            if (isDegree && (postupdirQ || postdowndirQ) && !postcontent.empty()) {
                Rend *postrend = new Rend();
                Text *posttext = new Text();
                postrend->AddChild(posttext);
                posttext->SetText(postcontent);
                if (postupdirQ) {
                    postrend->SetRend(TEXTRENDITION_sup);
                    if (postleapQ) {
                        setFontsize(postrend, "", "120%");
                        postrend->SetType("departure-up-leap");
                    }
                    else {
                        postrend->SetType("departure-up-step");
                    }
                }
                else if (postdowndirQ) {
                    postrend->SetRend(TEXTRENDITION_sub);
                    if (postleapQ) {
                        setFontsize(postrend, "", "120%");
                        postrend->SetType("departure-down-leap");
                    }
                    else {
                        postrend->SetType("departure-down-step");
                    }
                }

                if (rend) {
                    rend->AddChild(postrend);
                }
                else {
                    harm->AddChild(postrend);
                }
            }

            hum::HumNum tstamp = getMeasureTstamp(token, xstaffindex);
            harm->SetTstamp(tstamp.getFloat());
            std::string meilabel = token->getValue("auto", "meilabel");
            if (!meilabel.empty()) {
                addHarmLabel(harm, meilabel);
            }

            int aboveQ = token->getValueInt("auto", "above");
            if (aboveQ) {
                setPlaceRelStaff(harm, "above", false);
            }
            else {
                setPlaceRelStaff(harm, "below", false);
            }

            std::string fontsize = token->getValue("auto", "fontsize");
            if ((!fontsize.empty()) && (fontsize != "true") && (fontsize != "false")) {
                setFontsizeForHarm(harm, fontsize);
            }

            int boldQ = token->getValueInt("auto", "bold");
            if (boldQ) {
                setFontStyleForHarm(harm, "bold");
            }
            int italicQ = token->getValueInt("auto", "italic");
            if (italicQ) {
                setFontStyleForHarm(harm, "italic");
            }

            setLocationId(harm, token);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::setFontStyleFormHarm -- Add italic and/or bold font styling to harm data.
//     If there is more than one child, wrap all of them in a new rend with the font
//     style.  Also do this if the only child is not a rend.  If there is a single
//     rend, then apply the styling directly to that rend.
//

void HumdrumInput::setFontStyleForHarm(Harm *harm, const std::string &style)
{
    int childcount = harm->GetChildCount();
    Object *child = NULL;
    bool makeRendQ = false;
    if (childcount == 0) {
        return;
    }
    else if (childcount != 1) {
        makeRendQ = true;
    }
    else {
        // If the only child is a rend, then set the style of that rend.
        // If the only child is not a rend, then create a new rend and
        // have it adopt the child.
        child = harm->GetChild(0);
        if (!child) {
            return;
        }
        std::string childname = child->GetClassName();
        if (childname == "Rend") {
            if (style == "bold") {
                setFontWeight((Rend *)child, style);
            }
            else if (style == "italic") {
                setFontStyle((Rend *)child, style);
            }
            return;
        }
        else {
            makeRendQ = true;
        }
    }

    if (!makeRendQ) {
        return;
    }

    // Create a new rend to insert between harm and its children,
    // and then set the fontsize for the new rend.
    Rend *newrend = new Rend();

    // Transfer the children of harm to newrend:
    for (int i = 0; i < (int)childcount; i++) {
        Object *obj = harm->Relinquish(i);
        if (obj) {
            newrend->AddChild(obj);
        }
    }
    harm->ClearRelinquishedChildren();
    harm->AddChild(newrend);
    if (style == "bold") {
        setFontWeight(newrend, style);
    }
    else if (style == "italic") {
        setFontStyle(newrend, style);
    }
}

//////////////////////////////
//
// HumdrumInput::setFontWeight --
//

void HumdrumInput::setFontWeight(Rend *rend, const std::string &fontweight)
{
    rend->SetFontweight(rend->AttTypography::StrToFontweight(fontweight));
}

//////////////////////////////
//
// HumdrumInput::setFontsizeForHarm -- Add rend@fontsize to harm data.  If there is more than
//     one child for the harm, wrap all of them in a rend to set the fontsize.  If there
//     is no rend as the sole child of the harm, then wrap the text content of harm in
//     a rend to set fontsize.  May be made a template in the future.  Also would be
//     useful to validate the input fontsize string.
//

void HumdrumInput::setFontsizeForHarm(Harm *harm, const std::string &fontsize)
{
    int childcount = harm->GetChildCount();
    Object *child = NULL;
    bool makeRendQ = false;
    if (childcount == 0) {
        return;
    }
    else if (childcount != 1) {
        makeRendQ = true;
    }
    else {
        // If the only child is a rend, then check if the fontsize is set.
        // If it is set, then insert a new rend that will adopt the child; otherwise,
        // do not create a new rend, and set the existing rend's fontsize.
        // If the only child is not a rend, then create a rend and have it
        // adopt the child.
        child = harm->GetChild(0);
        if (!child) {
            return;
        }
        std::string childname = child->GetClassName();
        if (childname == "Rend") {
            bool emptyfontstyle = child->HasAttribute("fontstyle", "");
            if (emptyfontstyle) {
                setFontsize((Rend *)child, "", fontsize);
                return;
            }
            else {
                makeRendQ = true;
            }
        }
        else {
            makeRendQ = true;
        }
    }

    if (!makeRendQ) {
        return;
    }

    // Create a new rend to insert between harm and its children,
    // and then set the fontsize for the new rend.
    Rend *newrend = new Rend();

    // Transfer the children of harm to newrend:
    for (int i = 0; i < (int)childcount; i++) {
        Object *obj = harm->Relinquish(i);
        if (obj) {
            newrend->AddChild(obj);
        }
    }
    harm->ClearRelinquishedChildren();
    harm->AddChild(newrend);
    setFontsize(newrend, "", fontsize);
}

//////////////////////////////
//
// HumdrumInput::addHarmLabel --
//

void HumdrumInput::addHarmLabel(Harm *harm, const std::string &label)
{
    cerr << "ADD HARM LABEL " << label << " HERE" << endl;
}

//////////////////////////////
//
// HumdrumInput::getTrackText --
//

std::string HumdrumInput::getTrackText(hum::HTp token)
{
    std::string trackinfo = token->getSpineInfo();
    int track = token->getTrack();
    std::string output = to_string(track);
    std::string extension = "";
    for (int i = 0; i < (int)trackinfo.size(); ++i) {
        if (trackinfo[i] == 'a') {
            extension += 'a';
        }
        else if (trackinfo[i] == 'b') {
            extension += 'b';
        }
    }
    if (extension != "a") {
        output += extension;
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::splitFBString --
//    default value: separator = " "
//

std::vector<std::string> HumdrumInput::splitFBString(const std::string &content, const std::string &separator)
{
    hum::HumRegex hre;
    std::vector<std::string> pieces;
    hre.split(pieces, content, separator);
    return pieces;
}

//////////////////////////////
//
// HumdrumInput::cleanFBString --
//

std::vector<std::u32string> HumdrumInput::cleanFBString(std::vector<std::string> &pieces, hum::HTp token)
{
    std::vector<bool> todelete(pieces.size(), false);
    std::vector<std::u32string> output(pieces.size());
    for (int i = 0; i < (int)pieces.size(); ++i) {
        output[i] = convertFBNumber(pieces[i], token);
        if ((pieces[i].find("K") != std::string::npos)
            && ((pieces[i].find("x") == std::string::npos) && (pieces[i].find("X") == std::string::npos))) {
            // this figure slot needs to be deleted
            todelete[i] = true;
        }
    }
    // delete unwanted slots.
    for (int i = (int)todelete.size() - 1; i >= 0; i--) {
        if (!todelete[i]) {
            continue;
        }
        output.erase(output.begin() + i);
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::cleanFBString2 -- Used to concantenate multiple figures
//   together with hyphens.
//

std::u32string HumdrumInput::cleanFBString2(std::vector<std::string> &pieces, hum::HTp token)
{
    std::u32string output;
    for (int i = 0; i < (int)pieces.size(); ++i) {
        output += convertFBNumber(pieces[i], token);
        if (i < (int)pieces.size() - 1) {
            if (pieces[i + 1] == "") {
                output += U" ";
            }
            output += U"-";
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::convertFBNumber --
//

std::u32string HumdrumInput::convertFBNumber(const std::string &input, hum::HTp token)
{
    std::u32string output;

    int track = token->getTrack();
    int reverse = m_reverse[track];

    if (input.find("K") != std::string::npos) {
        return output;
    }

    bool found = false;
    int digit = 0;
    for (int i = 0; i < (int)input.size(); ++i) {
        if (isdigit(input[i])) {
            // digits have to be adjacent
            found = true;
            if ((i > 0) && isdigit(input[i - 1])) {
                digit = 10 * digit + (input[i] - '0');
            }
            else {
                digit = input[i] - '0';
            }
        }
    }
    if (!found) {
        digit = -1;
    }

    int slash = 0;
    // slash = 1 :: forward slash /
    // slash = 2 :: back slash \ .
    // slash = 3 :: vertical slash |
    if (input.find("/") != std::string::npos) {
        slash = 1;
    }
    else if (input.find("\\") != std::string::npos) {
        slash = 2;
    }
    else if (input.find("|") != std::string::npos) {
        slash = 3;
    }

    if (!m_slash[track]) {
        // disable slashes
        slash = 0;
    }

    if (digit > 9) {
        // Don't allow slashes on multi-digit numbers.
        // Maybe allow in the future.
        slash = 0;
    }

    int accidental = 0;
    // accidental = 1 :: double-flat
    // accidental = 2 :: flat (or lower)
    // accidental = 3 :: natural
    // accidental = 4 :: sharp (or raise)
    // accidental = 5 :: double-sharp
    // accidental = 6 :: plus (sharp or raise))
    // also allow "+" later
    if (input.find("+") != std::string::npos) {
        accidental = 6;
    }
    else if (input.find("--") != std::string::npos) {
        accidental = 1;
    }
    else if (input.find("##") != std::string::npos) {
        accidental = 5;
    }
    else if (input.find("-") != std::string::npos) {
        accidental = 2;
    }
    else if (input.find("#") != std::string::npos) {
        accidental = 4;
    }
    else if (input.find("n") != std::string::npos) {
        // other accidentals have priority over "n"
        // since they would be visual accidentals in the
        // case where "n" is also present.
        accidental = 3;
    }

    // Override visual accidentals if m_absolute is true:
    if (m_absolute[track]) {
        if (input.find("n") != std::string::npos) {
            accidental = 3;
        }
    }

    if (input.find("k") != std::string::npos) {
        // suppress display of accidental if "k" signifier is present.
        accidental = 0;
    }

    if (input.find("~") != std::string::npos) {
        // display minus sign for negative numbers if "~" signifier is present
        output += '-';
    }

    // accidental in front of number unless an "r" is present:
    if ((!slash) && (input.find("r") == std::string::npos) && (!reverse)) {
        std::u32string accid = getVisualFBAccidental(accidental);
        if (accidental && (input.find("i") != std::string::npos)) {
            accid = U"[" + accid + U"]";
        }
        else if (accidental && (input.find("j") != std::string::npos)) {
            accid = U"(" + accid + U")";
        }
        output += accid;
    }

    if (!slash) {
        // print regular number, slashed number must have accidental qualifier
        output += convertNumberToWstring(digit);
    }
    else {
        // slash should be drawn on number (but some numbers
        // do not has slashes available).
        // See: https://www.smufl.org/version/latest/range/figuredBass
        switch (digit) {
            case 0: output += U"\uEA50"; break; // draw without slash
            case 1: output += U"\uEA51"; break; // draw without slash
            case 2: output += U"\uEA53"; break;
            case 3: output += U"\uEA54"; break; // draw without slash
            case 4: output += U"\uEA56"; break; // only one style of slash
            case 5:
                switch (slash) {
                    case 1: output += U"\uEA5A"; break; // 5/
                    case 2: output += U"\uEA59"; break; // 5\ .
                    case 3: output += U"\uEA58"; break; // 5|
                    default: output += U"\uEA57"; break; // 5
                }
                break;
            case 6:
                switch (slash) {
                    case 1: output += U"\uEA5C"; break; // 6/
                    case 2: output += U"\uEA5C"; break; // 6\ .
                    case 3: output += U"\uEA5C"; break; // 6|
                    default: output += U"\uEA5B"; break; // 6
                }
                break;
            case 7:
                switch (slash) {
                    case 1: output += U"\uECC0"; break; // 7/
                    case 2: output += U"\uEA5F"; break; // 7\ .
                    case 3: output += U"\uEA5E"; break; // 7|
                    default: output += U"\uEA5D"; break; // 7
                }
                break;
            case 8: output += U"\uEA60"; break; // draw without slash
            case 9: output += U"\uEA62"; break; // only one style of slash
        }
    }

    // accidental after number if an "r" is present:
    if ((!slash) && ((input.find("r") != std::string::npos) || reverse)) {
        std::u32string accid = getVisualFBAccidental(accidental);
        if (accidental && (input.find("i") != std::string::npos)) {
            accid = U"[" + accid + U"]";
        }
        else if (accidental && (input.find("j") != std::string::npos)) {
            accid = U"(" + accid + U")";
        }
        output += accid;
    }

    if (input.find("J") != std::string::npos) {
        output = U"(" + output + U")";
    }
    else if (input.find("I") != std::string::npos) {
        output = U"[" + output + U"]";
    }
    else if (slash) {
        if (input.find("j") != std::string::npos) {
            output = U"(" + output + U")";
        }
        else if (input.find("i") != std::string::npos) {
            output = U"[" + output + U"]";
        }
    }

    // extension lines are not yet available for figured bass,
    // so display an underscore after the figure to indicate
    // that one should be added in the future:
    if (input.find("_") != std::string::npos) {
        output += U" _";
    }
    // A "=" character indicates that there is a the figure (should be
    // centered between current figure and next one, but not available yet).
    // Technically this is a sort of extender, but f@extension is
    // boolean, so various styles cannot be encoded in it.
    if (input.find("=") != std::string::npos) {
        output += U" -";
    }

    /*
                To convert a free-form std::string to UTF32:
                std::string tdee;
                output.back() += UTF8to32(tdee);
    */

    return output;
}

//////////////////////////////
//
// HumdrumInput::convertNumberToWstring --
//

std::u32string HumdrumInput::convertNumberToWstring(int number)
{
    if (number < 0) {
        return U"";
    }
    std::string value = to_string(number);
    std::u32string output;
    for (int i = 0; i < (int)value.size(); ++i) {
        switch (value[i]) {
            case '0': output += U"\uEA50"; break;
            case '1': output += U"\uEA51"; break;
            case '2': output += U"\uEA52"; break;
            case '3': output += U"\uEA54"; break;
            case '4': output += U"\uEA55"; break;
            case '5': output += U"\uEA57"; break;
            case '6': output += U"\uEA5B"; break;
            case '7': output += U"\uEA5D"; break;
            case '8': output += U"\uEA60"; break;
            case '9': output += U"\uEA61"; break;
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::getVisualFBAccidental --
//

std::u32string HumdrumInput::getVisualFBAccidental(int accidental)
{
    std::u32string output;
    switch (accidental) {
        case 1:
            output = U"\uE264"; // SMUFL double-flat
            break;
        case 2:
            // output = U"\u266D"; // unicode flat
            output = U"\uE260"; // SMUFL flat
            break;
        case 3:
            // output = U"\u266E"; // unicode natural
            output = U"\uE261"; // SMUFL natural
            break;
        case 4:
            // output = U"\u266F"; // unicode sharp
            output = U"\uE262"; // SMUFL sharp
            break;
        case 5:
            output = U"\uE263"; // SMUFL double-sharp
            break;
        case 6:
            output = U"+"; // UTF-7 +
            break;
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::cleanDegreeString --
//    n = 0 : the entire degree chord (default: currently mapped to n = 1)
//    n = 1 : the first degree in a chord
//    n = 2 : the second degree in a chord
//    n = 3 : the third degree in a chord
//    etc.
//

std::u32string HumdrumInput::cleanDegreeString(hum::HTp token, int n)
{
    std::string firstnote = *token;
    size_t spacepos = firstnote.find(" ");
    if (spacepos != std::string::npos) {
        firstnote.resize(spacepos + 1);
    }
    int sharps = 0;
    int flats = 0;
    for (int i = 0; i < (int)firstnote.size(); i++) {
        switch (firstnote[i]) {
            case '+': sharps++; break;
            case '-': flats++; break;
        }
    }

    // Adjust the 7th scale degree in minor keys according
    // to if harmonic minor or natural minor is the display system.
    int minor = token->getValueInt("auto", "minor");
    if (minor && (firstnote.find("7") != std::string::npos)) {
        int minnat = token->getValueInt("auto", "minnat");
        int HQ = firstnote.find("H") != std::string::npos ? 1 : 0;
        int NQ = firstnote.find("N") != std::string::npos ? 1 : 0;
        if (minnat) {
            if (HQ) {
                if (flats) {
                    flats--;
                }
                else {
                    sharps++;
                }
            }
        }
        else {
            // displaying in harmonic minor mode (default)
            if (NQ) {
                if (sharps) {
                    sharps--;
                }
                else {
                    flats++;
                }
            }
        }
    }

    std::u32string output;

    bool solfegeQ = token->getValueInt("auto", "solf");
    int accidQ = !token->getValueInt("auto", "nodegacc");
    int arrowQ = token->getValueInt("auto", "arrow");
    int revaccQ = token->getValueInt("auto", "accR");
    int revarrQ = token->getValueInt("auto", "arrR");

    if (arrowQ && revarrQ && !revaccQ) {
        output += addSemitoneAdjustmentsToDeg(token, arrowQ, accidQ, solfegeQ, sharps, flats);
    }
    else if (!arrowQ && !revaccQ) {
        output += addSemitoneAdjustmentsToDeg(token, arrowQ, accidQ, solfegeQ, sharps, flats);
    }

    hum::HumRegex hre;
    if (hre.search(firstnote, "(\\d+)")) {
        int degree = hre.getMatchInt(1);
        int semitones = sharps;
        if (flats) {
            semitones = -flats;
        }
        if (solfegeQ) {
            output += getMoveableDoName(token, degree, semitones);
        }
        else {
            switch (degree) {
                case 0: output += U"0"; break;
                case 1: output += U"1"; break;
                case 2: output += U"2"; break;
                case 3: output += U"3"; break;
                case 4: output += U"4"; break;
                case 5: output += U"5"; break;
                case 6: output += U"6"; break;
                case 7: output += U"7"; break;
                case 8: output += U"8"; break;
                case 9: output += U"9"; break;
            }
            if (token->getValueInt("auto", "hat")) {
                output += U"\u0302";
            }
        }
    }

    if (arrowQ && !revaccQ && !revarrQ) {
        output += addSemitoneAdjustmentsToDeg(token, arrowQ, accidQ, solfegeQ, sharps, flats);
    }
    else if (!arrowQ && revaccQ) {
        output += addSemitoneAdjustmentsToDeg(token, arrowQ, accidQ, solfegeQ, sharps, flats);
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::addSemitoneAdjustmentsToDeg --
//

std::u32string HumdrumInput::addSemitoneAdjustmentsToDeg(
    hum::HTp token, int arrowQ, int accidQ, int solfegeQ, int sharps, int flats)
{

    std::u32string output;

    // Add semitone adjustments
    if (accidQ && !solfegeQ) {
        if (sharps > 0) {
            if (sharps == 1) {
                if (arrowQ) {
                    output += U"\u2191"; // up arrow
                }
                else {
                    output += U"\u266f"; // sharp
                }
            }
            else if (sharps == 2) {
                if (arrowQ) {
                    output += U"\u21D1"; // double up arrow
                    // output += U"\u21C8"; // double up arrow
                }
                else {
                    output += U"\u266f\u266f"; // two sharps
                }
            }
            else {
                for (int i = 0; i < sharps; i++) {
                    if (arrowQ) {
                        output += U"\u2191"; // up arrow
                    }
                    else {
                        output += U"\u266f"; // sharp
                    }
                }
            }
        }
        else if (flats > 0) {
            if (flats == 1) {
                if (arrowQ) {
                    output += U"\u2193"; // down arrow
                }
                else {
                    output += U"\u266d"; // flat
                }
            }
            else if (flats == 2) {
                if (arrowQ) {
                    output += U"\u21D3"; // double down arrow
                    // output += U"\u21CA"; // double down arrow
                }
                else {
                    output += U"\u266d\u266d"; // two flats
                }
            }
            else {
                for (int i = 0; i < flats; i++) {
                    if (arrowQ) {
                        output += U"\u2193"; // down arrow
                    }
                    else {
                        output += U"\u266d"; // flat
                    }
                }
            }
        }
    }
    return output;
}

/////////////////////////////
//
// HumdrumInput::getMoveableDoName --  Up to +/- 2 semitone alteration.
//

std::u32string HumdrumInput::getMoveableDoName(hum::HTp token, int degree, int alteration)
{
    int minorQ = token->getValueInt("auto", "minor");
    if (minorQ && ((degree == 3) || (degree == 6))) {
        // adjust to major scale since solfege is absolute pitch
        alteration--;
    }
    int minnat = token->getValueInt("auto", "minnat");
    if (minnat) {
        if (token->find("7N") != std::string::npos) {
            alteration--;
        }
        else if (token->find("7H") != std::string::npos) {
            // do nothing
        }
        else if (token->find("7") != std::string::npos) {
            alteration--;
        }
    }

    int deg = (degree + 700 - 1) % 7; // zero-index degree and limit to an octave

    switch (deg) {
        case 0: // do
            switch (alteration) {
                case -2: return U"te";
                case -1: return U"ti";
                case 0: return U"do";
                case +1: return U"di";
                case +2: return U"re";
            }
            break;

        case 1: // re
            switch (alteration) {
                case -2: return U"do";
                case -1: return U"ra";
                case 0: return U"re";
                case +1: return U"ri";
                case +2: return U"mi";
            }
            break;

        case 2: // mi
            switch (alteration) {
                case -2: return U"re";
                case -1: return U"me";
                case 0: return U"mi";
                case +1: return U"fa";
                case +2: return U"fe";
            }
            break;

        case 3: // fa
            switch (alteration) {
                case -2: return U"me";
                case -1: return U"mi";
                case 0: return U"fa";
                case +1: return U"fi";
                case +2: return U"so";
            }
            break;

        case 4: // sol
            switch (alteration) {
                case -2: return U"fa";
                case -1: return U"se";
                case 0: return U"so";
                case +1: return U"si";
                case +2: return U"la";
            }
            break;

        case 5: // la
            switch (alteration) {
                case -2: return U"so";
                case -1: return U"le";
                case 0: return U"la";
                case +1: return U"li";
                case +2: return U"ti";
            }
            break;

        case 6: // ti
            switch (alteration) {
                case -2: return U"la";
                case -1: return U"te";
                case 0: return U"ti";
                case +1: return U"do";
                case +2: return U"di";
            }
            break;
    }

    return U"?";
}

//////////////////////////////
//
// HumdrumInput::cleanHarmString3 -- Adjust **rhrm text to remove
//     **recip data and then clean with cleanHarmString2.
//     **rhrm cannot contain alternate chords using [vi] syntax,
//     because [, _, ], are for tied notes, **rhrm tokens with _ or ]
//     will be suppressed.
//

std::u32string HumdrumInput::cleanHarmString3(const std::string &content)
{
    std::string temp;

    // hide **rhrm token if not a harmony "attack":

    if (content.find("_") != std::string::npos) {
        return U"";
    }
    if (content.find("]") != std::string::npos) {
        return U"";
    }

    // skip over **recip data:
    int i;
    for (i = 0; i < (int)content.size(); ++i) {
        if ((content[i] == '-') || (content[i] == '#')) {
            break;
        }
        if (isalpha(content[i])) {
            // V, I, ii, vi, Lt, Gn, Fr, N
            break;
        }
    }

    int foundstartsquare = false;
    for (int ii = i; ii < (int)content.size(); ++ii) {
        if (content[ii] == '[') {
            foundstartsquare = true;
        }
        if (content[ii] == ']') {
            if (!foundstartsquare) {
                continue;
            }
        }
        if (content[ii] == '_') {
            continue;
        }
        temp += content[i];
    }

    return cleanHarmString2(temp);
}

//////////////////////////////
//
// HumdrumInput::cleanHarmString2 -- Adjust **harm text
//

std::u32string HumdrumInput::cleanHarmString2(const std::string &content)
{
    std::u32string output;
    bool nonrhythm = false;
    for (int i = 0; i < (int)content.size(); ++i) {
        if (!nonrhythm) {
            if (isdigit(content[i])) {
                continue;
            }
            if (content[i] == '%') {
                continue;
            }
            if (isdigit(content[i] == '.')) {
                continue;
            }
        }
        nonrhythm = true;
        if (content[i] == '-') {
            output += U"\u266D"; // unicode flat
        }
        else if (content[i] == '#') {
            output += U"\u266F"; // unicode sharp
        }
        else if (content[i] == 'D') {
            output += U"\u00F8"; // o-slash
        }
        else if (content[i] == 'o') {
            output += U"\u00B0"; // degree sign
        }
        else {
            std::string tdee;
            tdee = content[i];
            output += UTF8to32(tdee);
        }
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::cleanStringString -- Add circles around std::string numbers.
//    see: https://www.fileformat.info/info/unicode/block/enclosed_alphanumerics/utf8test.htm
//

std::u32string HumdrumInput::cleanStringString(const std::string &content)
{
    std::u32string output;
    std::string value;
    for (int i = 0; i < (int)content.size(); ++i) {
        switch (content[i]) {
            case '0': output += U"\u24ea"; break; // 0 in circle
            case '1': output += U"\u2460"; break; // 1 in circle
            case '2': output += U"\u2461"; break; // 2 in circle
            case '3': output += U"\u2462"; break; // 3 in circle
            case '4': output += U"\u2463"; break; // 4 in circle
            case '5': output += U"\u2464"; break; // 5 in circle
            case '6': output += U"\u2465"; break; // 6 in circle
            case '7': output += U"\u2466"; break; // 7 in circle
            case '8': output += U"\u2467"; break; // 8 in circle
            case '9': output += U"\u2468"; break; // 9 in circle
            default:
                value.clear();
                value.push_back(content[i]);
                output += UTF8to32(value);
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::cleanHarmString --
//

std::u32string HumdrumInput::cleanHarmString(const std::string &content)
{
    std::u32string root;
    std::u32string kind;
    std::u32string bass;

    bool foundspace = false;
    bool foundslash = false;
    for (int i = 0; i < (int)content.size(); ++i) {
        if (content[i] == '/') {
            foundslash = true;
        }
        if (foundspace && !foundslash) {
            std::string tdee;
            tdee = content[i];
            kind += UTF8to32(tdee);
            continue;
        }
        if (content[i] == ' ') {
            foundspace = true;
            continue;
        }
        if (content[i] == '/') {
            foundslash = true;
            continue;
        }
        if (!foundspace) {
            if (content[i] == '-') {
                root += U"\u266D"; // unicode flat
            }
            else if (content[i] == '#') {
                root += U"\u266F"; // unicode sharp
            }
            else {
                std::string tdee;
                tdee = content[i];
                root += UTF8to32(tdee);
            }
        }
        else if (foundslash) {
            if (content[i] == '-') {
                bass += U"\u266D"; // unicode flat
            }
            else if (content[i] == '#') {
                bass += U"\u266F"; // unicode sharp
            }
            else {
                std::string tdee;
                tdee = content[i];
                bass += UTF8to32(tdee);
            }
        }
        else {
            cerr << "should not get here with correct input " << content << endl;
        }
    }

    bool replacing = false;
    if (kind == U"major-minor") {
        kind = U"Mm7";
        replacing = true;
    }
    else if (kind == U"minor-major") {
        kind = U"mM7";
        replacing = true;
    }

    if (replace(kind, U"major-", U"maj")) {
        replacing = true;
    }
    else if (replace(kind, U"minor-", U"m")) {
        replacing = true;
    }
    else if (replace(kind, U"dominant-", U"dom")) {
        replacing = true;
    }
    else if (replace(kind, U"augmented-", U"+")) {
        replacing = true;
    }
    else if (replace(kind, U"suspended-", U"sus")) {
        replacing = true;
    }
    else if (replace(kind, U"diminished-", U"\u00B0")) { // degree sign
        replacing = true;
    }
    if (replace(kind, U"seventh", U"7")) {
        replacing = true;
    }
    else if (replace(kind, U"ninth", U"9")) {
        replacing = true;
    }
    else if (replace(kind, U"11th", U"11")) {
        replacing = true;
    }
    else if (replace(kind, U"13th", U"13")) {
        replacing = true;
    }
    else if (replace(kind, U"second", U"2")) {
        replacing = true;
    }
    else if (replace(kind, U"fourth", U"4")) {
        replacing = true;
    }
    else if (replace(kind, U"sixth", U"6")) {
        replacing = true;
    }

    if (kind == U"major") {
        kind = U"";
        replacing = true;
    }
    else if (kind == U"maj") {
        kind = U"";
        replacing = true;
    }
    else if (kind == U"ma") {
        kind = U""; // degree sign
        replacing = true;
    }
    else if (kind == U"minor") {
        kind = U"m";
        replacing = true;
    }
    else if (kind == U"min") {
        kind = U"m";
        replacing = true;
    }
    else if (kind == U"augmented") {
        kind = U"+";
        replacing = true;
    }
    else if (kind == U"minor-seventh") {
        kind = U"m7";
        replacing = true;
    }
    else if (kind == U"major-seventh") {
        kind = U"maj7";
        replacing = true;
    }
    else if (kind == U"dom11") {
        kind = U"11";
        replacing = true;
    }
    else if (kind == U"dom13") {
        kind = U"13";
        replacing = true;
    }
    else if (kind == U"dom9") {
        kind = U"9";
        replacing = true;
    }
    else if (kind == U"half-diminished") {
        kind = U"\u00F8"; // o-slash
        replacing = true;
    }
    else if (kind == U"diminished") {
        kind = U"\u00B0"; // degree sign
        replacing = true;
    }
    else if (kind == U"dominant") {
        kind = U"7";
        replacing = true;
    }
    else if (kind == U"power") {
        kind = U"5";
        replacing = true;
    }
    else if (kind == U"m7b5") {
        replacing = true;
        kind = U"m7\u266D"
               U"5";
    }
    if ((kind != U"") && !replacing) {
        root += U' ';
    }
    if (bass != U"") {
        kind += U'/';
    }
    std::u32string output = root + kind + bass;
    return output;
}

//////////////////////////////
//
// HumdrumInput::replace -- simple substring replacement implementation.
//    Returns true if there was any replacment done.
//

bool HumdrumInput::replace(string &str, const std::string &oldStr, const std::string &newStr)
{
    std::string::size_type pos = 0u;
    bool output = false;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        output = true;
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return output;
}

bool HumdrumInput::replace(std::u32string &str, const std::u32string &oldStr, const std::u32string &newStr)
{
    std::u32string::size_type pos = 0u;
    bool output = false;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        output = true;
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::convertMeasureStaff -- print a particular staff in a
//     particular measure.
//

bool HumdrumInput::convertMeasureStaff(int track, int startline, int endline, int n, int layercount)
{
    bool status = true;
    m_clef_buffer.clear();

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_currentstaff - 1;

    for (int i = 0; i < layercount; ++i) {
        m_join = ss.at(staffindex).join;
        status &= convertStaffLayer(track, startline, endline, i);
        if (!status) {
            break;
        }
    }
    if (ss.at(staffindex).glissStarts.size() > 0) {
        insertGlissandos(ss.at(staffindex).glissStarts);
    }
    ss.at(staffindex).join = m_join;
    checkClefBufferForSameAs();
    return status;
}

//////////////////////////////
//
// HumdrumInput::checkClefBufferForSameAs -- identify clefs that should not be printed.
//     This information was calculated when processing the layers, so now check through
//     the list of clefs to see if any are marked for suppression.
//
// tuple elements:
//    0 = boolean (true = bad)
//    1 = HumNum timestamp of clef
//    2 = Clef*
//

void HumdrumInput::checkClefBufferForSameAs()
{
    for (int i = 0; i < (int)m_clef_buffer.size(); ++i) {
        if (std::get<0>(m_clef_buffer[i])) {
            suppressBufferedClef(i);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::suppressBufferedClef -- Mark the given class as "sameas" provided that
//     there is another clef at the same timestamp that is not also a "sameas" clef.
//     The input index is the position of the bad clef in the buffered list for the
//     staff measure.
//

void HumdrumInput::suppressBufferedClef(int index)
{
    hum::HumNum target = std::get<1>(m_clef_buffer.at(index));
    Clef *goodclef = NULL;
    for (int i = 0; i < (int)m_clef_buffer.size(); ++i) {
        if (std::get<0>(m_clef_buffer[i])) {
            // don't look at bad clefs
            continue;
        }
        if (target == std::get<1>(m_clef_buffer[i])) {
            goodclef = std::get<2>(m_clef_buffer[i]);
            break;
        }
    }
    if (!goodclef) {
        return;
    }

    Clef *badclef = std::get<2>(m_clef_buffer.at(index));
    if (!badclef) {
        return;
    }

    badclef->SetSameas("#" + goodclef->GetID());
}

//////////////////////////////
//
// HumdrumInput::convertStaffLayer -- Prepare a layer element in the current
//   staff and then fill it with data.
//

bool HumdrumInput::convertStaffLayer(int track, int startline, int endline, int layerindex)
{
    m_layer = new Layer();
    m_currentlayer = layerindex + 1;

    m_layer->SetN(layerindex + 1);
    m_staff->AddChild(m_layer);

    std::vector<int> &rkern = m_rkern;
    int staffindex = rkern[track];
    std::vector<hum::HTp> &layerdata = m_layertokens[staffindex][layerindex];

    if (layerdata.size() > 0) {
        if (layerdata[0]->size() > 0) {
            setLocationIdNSuffix(m_layer, layerdata[0], layerindex + 1);
            // Start the Layer at startline rather than line of first token in layer.
            std::string id = m_layer->GetID();
            hum::HumRegex hre;
            std::string replacement = "L";
            replacement += to_string(startline + 1);
            hre.replaceDestructive(id, replacement, "L\\d+");
            m_layer->SetID(id);
        }
    }

    if ((layerindex == 0) && (layerdata.size() > 0)) {
        if ((layerdata[0]->size() > 0) && (layerdata[0]->at(0) == '=')) {
            setLocationIdNSuffix(m_staff, layerdata[0], layerindex + 1);
        }
    }

    if (m_comment) {
        std::string comment;
        comment += " kern: ";
        for (int i = 0; i < (int)layerdata.size(); ++i) {
            comment += *layerdata[i];
            if (i < (int)layerdata.size() - 1) {
                comment += "  ";
            }
        }
        comment += " ";
        m_layer->SetComment(comment);
    }

    return fillContentsOfLayer(track, startline, endline, layerindex);
}

//////////////////////////////
//
// HumdrumInput::fixLargeTuplets -- fix triple-breve/triplet-wholenote cases.
//

void HumdrumInput::fixLargeTuplets(std::vector<humaux::HumdrumBeamAndTuplet> &tg)
{
    // triplet-whole + triplet-breve cases
    for (int i = 1; i < (int)tg.size(); ++i) {
        if ((tg.at(i).tupletstart == 2) && (tg.at(i).tupletend == 1) && (tg.at(i - 1).tupletstart == 1)
            && (tg.at(i - 1).tupletend == 1)) {
            tg.at(i).tupletstart = 0;
            tg.at(i - 1).tupletend = 0;
        }
    }

    // two triplet-halfs + triplet-breve case
    for (int i = 2; i < (int)tg.size(); ++i) {
        if ((tg.at(i).tupletstart == 2) && (tg.at(i).tupletend == 1) && (tg.at(i - 1).tupletstart == 0)
            && (tg.at(i - 1).tupletend == 1) && (tg.at(i - 2).tupletstart == 1) && (tg.at(i - 2).tupletend == 0)) {
            tg.at(i - 1).numscale = 1;
            tg.at(i - 2).numscale = 1;
            tg.at(i).tupletstart = 0;
            tg.at(i - 1).tupletend = 0;
            tg.at(i).numbase = 2;
        }
    }

    // two triplet-halfs + triplet-breve case + two triplet-halfs
    for (int i = 2; i < (int)tg.size(); ++i) {
        if ((tg.at(i).tupletstart == 0) && (tg.at(i).tupletend == 2) && (tg.at(i - 1).tupletstart == 2)
            && (tg.at(i - 1).tupletend == 0) && (tg.at(i - 2).tupletstart == 1) && (tg.at(i - 2).tupletend == 1)) {
            tg.at(i).tupletend = 1;
            tg.at(i - 1).tupletstart = 0;
            tg.at(i - 2).tupletend = 0;
            tg.at(i - 2).numbase = 2;
            tg.at(i).numscale = 1;
            tg.at(i - 1).numscale = 1;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::printGroupInfo --
//

void HumdrumInput::printGroupInfo(const std::vector<humaux::HumdrumBeamAndTuplet> &tg)
{
    cerr << "TOK\t\tGRP\tBRAK\tNUM\tNBASE\tNSCAL\tBSTART\tBEND";
    cerr << "\tGBST\tGBEND\tTSTART\tTEND\tFORCE\tPRIORITY\n";
    for (int i = 0; i < (int)tg.size(); ++i) {
        cerr << tg.at(i).token << "\t";
        if (tg.at(i).token && (tg.at(i).token->size() < 8)) {
            cerr << "\t";
        }
        cerr << tg.at(i).group << "\t";
        cerr << tg.at(i).bracket << "\t";
        cerr << tg.at(i).num << "\t";
        cerr << tg.at(i).numbase << "\t";
        cerr << tg.at(i).numscale << "\t";
        cerr << tg.at(i).beamstart << "\t";
        cerr << tg.at(i).beamend << "\t";
        cerr << tg.at(i).gbeamstart << "\t";
        cerr << tg.at(i).gbeamend << "\t";
        cerr << "TS:" << tg.at(i).tupletstart << "\t";
        cerr << "TE:" << tg.at(i).tupletend << "\t";
        cerr << tg.at(i).force << "\t";
        cerr << tg.at(i).priority;
        cerr << endl;
    }
    cerr << "============================================" << endl;
}

//////////////////////////////
//
// HumdrumInput::setBeamDirection -- Set a beam up or down.
//

void HumdrumInput::setBeamDirection(int direction, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs,
    std::vector<hum::HTp> &layerdata, int layerindex, bool grace)
{
    const humaux::HumdrumBeamAndTuplet &tg = tgs.at(layerindex);
    int beamstart = tg.beamstart;
    if (grace) {
        beamstart = tg.gbeamstart;
    }
    int beamend;

    for (int i = layerindex; i < (int)layerdata.size(); ++i) {
        beamend = tgs.at(i).beamend;
        if (grace) {
            beamend = tgs.at(i).gbeamend;
        }
        if (!layerdata[i]->isData()) {
            continue;
        }
        if (layerdata[i]->isNull()) {
            continue;
        }
        if (layerdata[i]->isRest()) {
            // not adding stem direction to rests
            continue;
        }
        if ((layerdata[i]->getDuration() == 0) && !grace) {
            // ignore grace note beams
            continue;
        }
        else if ((layerdata[i]->getDuration() != 0) && grace) {
            // ignore non-grace note beams
            continue;
        }
        layerdata[i]->setValue("auto", "stem.dir", to_string(direction));
        if (beamend == beamstart) {
            // last note of beam so exit
            break;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForTremolo --  Check to see if a beamed group of notes
//    can be converted into a tremolo. (Decision to convert to tremolo is done
//    outside of this function and is activated by the *tremolo tandem interpretation).
//

bool HumdrumInput::checkForTremolo(
    std::vector<hum::HTp> &layerdata, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs, int startindex)
{
    int beamnumber = tgs.at(startindex).beamstart;
    std::vector<hum::HTp> notes;
    for (int i = startindex; i < (int)layerdata.size(); ++i) {
        if (layerdata[i]->isNote()) {
            notes.push_back(layerdata[i]);
        }
        if (tgs.at(i).beamend == beamnumber) {
            break;
        }
    }
    if (notes.empty()) {
        return false;
    }

    hum::HumNum duration = notes[0]->getDuration();
    hum::HumNum testdur = duration;
    std::vector<std::vector<int>> pitches(notes.size());
    // std::vector<HumNum> durations(notes.size());

    bool firstHasTie = false;
    bool lastHasTie = false;
    for (int i = 0; i < (int)notes.size(); ++i) {
        if ((notes[i]->find('_') != std::string::npos) || (notes[i]->find('[') != std::string::npos)
            || (notes[i]->find(']') != std::string::npos)) {
            // Note/chord involved a tie is present,
            // so disallow any tremolo on this beamed group.
            if (i == 0) {
                firstHasTie = true;
            }
            else if (i == (int)notes.size() - 1) {
                lastHasTie = true;
            }
            else {
                return false;
            }
        }

        // durations.at(i) = notes[i]->getDuration();

        if (i > 0) {
            testdur = notes[i]->getDuration();
            if (testdur != duration) {
                // All durations in beam must be the same for a tremolo.
                // (at least for now).
                return false;
            }
        }

        // Store all notes in chord for comparing in next loop.
        int scount = notes[i]->getSubtokenCount();
        for (int j = 0; j < scount; j++) {
            std::string subtok = notes[i]->getSubtoken(j);
            pitches[i].emplace_back(hum::Convert::kernToBase40(subtok));
        }
    }

    // Check for <bTrem> case.
    std::vector<bool> nextsame(notes.size(), true);
    bool allpequal = true;
    if (firstHasTie || lastHasTie) {
        allpequal = false;
    }
    else {
        for (int i = 1; i < (int)pitches.size(); ++i) {
            if (pitches[i].size() != pitches[i - 1].size()) {
                allpequal = false;
                nextsame.at(i - 1) = false;
                // break;
            }
            // Check if each note in the successive chords is the same.
            // The ordering of notes in each chord is assumed to be the same
            // (i.e., this function is not going to waste time sorting
            // the pitches to check if the chords are equivalent).
            for (int j = 0; j < (int)pitches[i].size(); j++) {
                if (pitches[i][j] != pitches[i - 1][j]) {
                    allpequal = false;
                    nextsame.at(i - 1) = false;
                    // break;
                }
            }
            // if (allpequal == false) {
            //   break;
            //}
        }
    }

    if (allpequal) {
        // beam group should be converted into a <bTrem> tremolo
        hum::HumNum tdur = duration * (int)notes.size();
        std::string recip = hum::Convert::durationToRecip(tdur);

        int slashes = log(duration.getFloat()) / log(2.0);
        int noteslash = log(tdur.getFloat()) / log(2.0);
        if (noteslash < 0) {
            slashes = slashes - noteslash;
        }
        slashes = -slashes;
        if (slashes <= 0) {
            // something went wrong calculating durations.
            return false;
        }

        notes[0]->setValue("auto", "tremolo", "1");
        notes[0]->setValue("auto", "recip", recip);
        notes[0]->setValue("auto", "slashes", slashes);
        for (int i = 1; i < (int)notes.size(); ++i) {
            notes[i]->setValue("auto", "suppress", "1");
        }

        return true;
    }

    // Check for multiple bTrem embedded in single beam group.
    // The current requirement is that all subgroups must have the
    // same duration (this requirement can be loosened in the future
    // if necessary).
    bool hasInternalTrem = true;
    for (int i = 1; i < (int)nextsame.size() - 1; ++i) {
        if (nextsame.at(i) == 1) {
            continue;
        }
        if (nextsame.at(i - 1) == 0) {
            hasInternalTrem = false;
            break;
        }
        else if (nextsame.at(i + 1) == 0) {
            hasInternalTrem = false;
            break;
        }
    }
    if (nextsame.size() == 2) {
        if ((nextsame[0] == 0) && (nextsame[1] == 1)) {
            hasInternalTrem = false;
        }
    }

    // Group separate tremolo groups within a single beam.
    std::vector<std::vector<hum::HTp>> groupings;
    if (hasInternalTrem) {
        groupings.reserve(16);
        groupings.resize(1);
        groupings.back().push_back(notes[0]);
        for (int i = 0; i < (int)notes.size() - 1; ++i) {
            if (nextsame[i]) {
                groupings.back().push_back(notes[i + 1]);
            }
            else {
                groupings.resize(groupings.size() + 1);
                groupings.back().push_back(notes[i + 1]);
            }
        }
    }

    // Current requirement is that the internal tremolos are power-of-two
    // (deal with dotted internal tremolos as needed in the future).
    bool allpow2 = true;
    if (hasInternalTrem) {
        for (int i = 0; i < (int)groupings.size(); ++i) {
            hum::HumNum count = (int)groupings[i].size();
            if (!count.isPowerOfTwo()) {
                allpow2 = false;
                break;
            }
        }
    }

    if (hasInternalTrem && allpow2) {
        // Ready to mark internal bTrem configuration.

        // First suppress printing of all non-primary tremolo notes:
        for (int i = 0; i < (int)groupings.size(); ++i) {
            for (int j = 1; j < (int)groupings[i].size(); j++) {
                groupings[i][j]->setValue("auto", "suppress", "1");
            }
        }

        // Now add tremolo slash(es) on the first notes.

        for (int i = 0; i < (int)groupings.size(); ++i) {
            hum::HumNum tdur = duration * (int)groupings[i].size();
            std::string recip = hum::Convert::durationToRecip(tdur);
            int slashcount = -(int)(log2(duration.getFloat() / tdur.getFloat()));
            groupings[i][0]->setValue("auto", "tremolo", "1");
            groupings[i][0]->setValue("auto", "slashes", slashcount);
            groupings[i][0]->setValue("auto", "recip", recip);
        }

        // Preserve the beam on the group of tremolos.  The beam can
        // only be an eighth-note beam for now (this should be the
        // general rule for beamed tremolos).
        groupings.at(0).at(0)->setValue("auto", "tremoloBeam", "8");
        groupings.back().back()->setValue("auto", "tremoloBeam", "8");

        // returning false in order to keep the beam.
        return false;
    }

    // Check for <fTrem> case.
    // allowing odd-length sequences (3, 5, 7, etc) which can in theory
    // be represented by fTrem, but I have not see such cases.

    if (pitches.size() < 3) {
        // fTrem cannot exist on only two notes/chords.
        return false;
    }

    // check to see that all even notes/chords are the same
    for (int i = 2; i < (int)pitches.size(); ++i) {
        if (pitches[i].size() != pitches[i - 2].size()) {
            return false;
        }
        // Check if each note in the successive chords is the same.
        // The ordering of notes in each chord is assumed to be the same
        // (i.e., this function is not going to waste time sorting
        // the pitches to check if the chords are equivalent).
        for (int j = 0; j < (int)pitches[i].size(); j++) {
            if (pitches[i][j] != pitches[i - 2][j]) {
                return false;
            }
        }
    }

    // If got to this point, create an fTrem.

    hum::HumNum tdur = duration * (int)notes.size();
    std::string recip = hum::Convert::durationToRecip(tdur);
    std::string unitrecip = hum::Convert::durationToRecip(duration);

    // Eventually also allow calculating of beam.float
    // (mostly for styling half note tremolos).
    int beams = -log(duration.getFloat()) / log(2.0);
    if (beams <= 0) {
        // something went wrong calculating durations.
        cerr << "PROBLEM WITH TREMOLO2 CALCULATION: " << beams << endl;
        return false;
    }

    notes[0]->setValue("auto", "tremolo2", "1");
    notes[0]->setValue("auto", "recip", recip);
    notes[0]->setValue("auto", "unit", unitrecip); // problem if dotted...
    notes[0]->setValue("auto", "beams", beams);

    int lasti = (int)notes.size() - 1;
    notes[lasti]->setValue("auto", "tremoloAux", "1");
    notes[lasti]->setValue("auto", "recip", recip);

    for (int i = 1; i < (int)notes.size(); ++i) {
        notes[i]->setValue("auto", "suppress", "1");
    }

    return true;
}

//////////////////////////////
//
// HumdrumInput::checkForInvisibleBeam --  Not checking for interleaved grace notes
//    within regular note beams yet.  Does not deal with cross-bar beams.
//

bool HumdrumInput::checkForInvisibleBeam(
    Beam *beam, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs, int layerindex)
{
    int beamnum = tgs.at(layerindex).beamstart;
    for (int i = layerindex; i < (int)tgs.size(); ++i) {
        if (!tgs.at(i).token) {
            cerr << "WARNING in checkForInvisibleBeam: NULL token\n";
            return false;
        }
        int len = (int)tgs.at(i).token->size();
        if (len > 0) {
            if (tgs.at(i).token->at(0) == '*') {
                continue;
            }
            if (tgs.at(i).token->at(0) == '!') {
                continue;
            }
            if (tgs.at(i).token->at(0) == '=') {
                continue;
            }
        }
        else {
            // strange problem
            return false;
        }

        hum::HTp token = tgs.at(i).token;
        std::vector<std::string> subtoks = token->getSubtokens();
        for (int j = 0; j < (int)subtoks.size(); j++) {
            if (subtoks[j].find("yy") == std::string::npos) {
                return false;
            }
        }
        int beamend = tgs.at(i).beamend;
        if (beamend == beamnum) {
            break;
        }
    }

    // All notes in a beam are invisible, so mark the beam as type="invisible":
    beam->SetType("invisible");

    return true;
}

//////////////////////////////
//
// HumdrumInput::handleGroupStarts --
//

void HumdrumInput::handleGroupStarts(const std::vector<humaux::HumdrumBeamAndTuplet> &tgs,
    std::vector<std::string> &elements, std::vector<void *> &pointers, std::vector<hum::HTp> &layerdata, int layerindex)
{

    Beam *beam;
    const humaux::HumdrumBeamAndTuplet &tg = tgs.at(layerindex);
    hum::HTp token = layerdata[layerindex];
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_currentstaff - 1;

    if (ss[staffindex].tremolo) {
        if (token->find("L") != std::string::npos) {
            bool status = checkForTremolo(layerdata, tgs, layerindex);
            if (status) {
                // beamed group converted into tremolo
                return;
            }
        }
    }

    int direction = 0;
    if (tg.beamstart || tg.gbeamstart) {
        if (m_signifiers.above) {
            std::string pattern = "[LJKk]+";
            pattern.push_back(m_signifiers.above);
            if (regex_search(*token, regex(pattern))) {
                direction = 1;
            }
        }
        if (m_signifiers.below) {
            std::string pattern = "[LJKk]+";
            pattern.push_back(m_signifiers.below);
            if (regex_search(*token, regex(pattern))) {
                direction = -1;
            }
        }
        if (direction) {
            if (tg.beamstart) {
                setBeamDirection(direction, tgs, layerdata, layerindex, false);
            }
            else {
                setBeamDirection(direction, tgs, layerdata, layerindex, true);
            }
        }
    }

    if (tg.beamstart && tg.tupletstart) {
        if (tg.priority == 'T') {
            insertTuplet(elements, pointers, tgs, layerdata, layerindex, ss[staffindex].suppress_tuplet_number,
                ss[staffindex].suppress_tuplet_bracket);
            beam = insertBeam(elements, pointers, tg);

            checkForInvisibleBeam(beam, tgs, layerindex);
            if (direction) {
                appendTypeTag(beam, "placed");
            }
            checkBeamWith(beam, tgs, layerdata, layerindex);
            setBeamLocationId(beam, tgs, layerdata, layerindex);
            std::string id = beam->GetID();
            layerdata[layerindex]->setValue("auto", "beamid", id);
        }
        else {
            beam = insertBeam(elements, pointers, tg);
            setBeamLocationId(beam, tgs, layerdata, layerindex);
            std::string id = beam->GetID();
            layerdata[layerindex]->setValue("auto", "beamid", id);
            bool status = checkForBeamSameas(beam, layerdata, layerindex);
            if (status) {
                // remove beam from stack
                removeBeam(elements, pointers);
                return;
            }
            checkForBeamStemSameas(layerdata, layerindex);

            checkForInvisibleBeam(beam, tgs, layerindex);
            if (direction) {
                appendTypeTag(beam, "placed");
            }

            checkBeamWith(beam, tgs, layerdata, layerindex);
            insertTuplet(elements, pointers, tgs, layerdata, layerindex, ss[staffindex].suppress_tuplet_number,
                ss[staffindex].suppress_tuplet_bracket);
        }
    }
    else if (tg.beamstart) {
        beam = insertBeam(elements, pointers, tg);
        setBeamLocationId(beam, tgs, layerdata, layerindex);
        std::string id = beam->GetID();
        layerdata[layerindex]->setValue("auto", "beamid", id);
        bool status = checkForBeamSameas(beam, layerdata, layerindex);
        if (status) {
            // remove beam from stack
            removeBeam(elements, pointers);
            return;
        }
        checkForBeamStemSameas(layerdata, layerindex);

        checkForInvisibleBeam(beam, tgs, layerindex);
        if (direction) {
            appendTypeTag(beam, "placed");
        }
        checkBeamWith(beam, tgs, layerdata, layerindex);
    }
    else if (tg.tupletstart) {
        insertTuplet(elements, pointers, tgs, layerdata, layerindex, ss[staffindex].suppress_tuplet_number,
            ss[staffindex].suppress_tuplet_bracket);
    }

    if (tg.gbeamstart) {
        // Grace note beams should not interact with
        // regular beams or tuplets.
        beam = insertGBeam(elements, pointers, tg);
        if (direction) {
            appendTypeTag(beam, "placed");
        }
        checkBeamWith(beam, tgs, layerdata, layerindex);
        setBeamLocationId(beam, tgs, layerdata, layerindex);
        std::string id = beam->GetID();
        layerdata[layerindex]->setValue("auto", "beamid", id);
    }
}

//////////////////////////////
//
// HumdrumInput::checkForBeamStemSameas -- Check to see if two beams
//     have the same rhythm sequence (but can have differen pitches).
//     If there are rests, they must align in both beams.
//

bool HumdrumInput::checkForBeamStemSameas(std::vector<hum::HTp> &layerdata, int layerindex)
{
    if (!m_join) {
        return false;
    }
    hum::HTp token = layerdata.at(layerindex);
    int subtrack = token->getSubtrack();
    if (subtrack != 2) {
        return false;
    }
    hum::HTp ptoken = token->getPreviousFieldToken();
    if (!ptoken) {
        return false;
    }
    if (ptoken->isNull()) {
        return false;
    }
    int ptrack = ptoken->getTrack();
    int track = token->getTrack();
    if (ptrack != track) {
        return false;
    }
    int beamstart1 = token->getValueInt("auto", "beamstart");
    int beamstart2 = ptoken->getValueInt("auto", "beamstart");
    if (beamstart1 == 0) {
        return false;
    }
    if (beamstart2 == 0) {
        return false;
    }

    // Finished with possible error cases, so now do real checking.

    std::vector<hum::HTp> data1 = getBeamNotes(token, beamstart1);
    std::vector<hum::HTp> data2 = getBeamNotes(ptoken, beamstart2);

    bool status = true;

    if (data1.size() != data2.size()) {
        status = false;
    }
    if (data1.empty()) {
        status = false;
    }

    if (status) {
        for (int i = 0; i < (int)data1.size(); ++i) {
            hum::HumNum dur1 = data1[i]->getDuration();
            hum::HumNum dur2 = data2[i]->getDuration();
            if (dur1 != dur2) {
                status = false;
                break;
            }
            if (data1[i]->isChord()) {
                status = false;
                break;
            }
            if (data2[i]->isChord()) {
                status = false;
                break;
            }
            if (data1[i]->isRest() && !data2[i]->isRest()) {
                status = false;
                break;
            }
            if (data2[i]->isRest() && !data1[i]->isRest()) {
                status = false;
                break;
            }
        }
    }

    if (status) {
        return true;
    }

    // Prevent notes from merging between the two parts.
    for (int i = 0; i < (int)data1.size(); ++i) {
        data1[i]->setValue("auto", "Xjoin", 1);
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::checkForBeamSameas -- Check to see if both beams are
//    equivalent in pitch and rhythm content.
//

bool HumdrumInput::checkForBeamSameas(Beam *beam, std::vector<hum::HTp> &layerdata, int layerindex)
{
    if (!m_join) {
        return false;
    }
    hum::HTp token = layerdata.at(layerindex);
    int subtrack = token->getSubtrack();
    if (subtrack != 2) {
        return false;
    }
    hum::HTp ptoken = token->getPreviousFieldToken();
    if (!ptoken) {
        return false;
    }
    if (ptoken->isNull()) {
        return false;
    }
    int ptrack = ptoken->getTrack();
    int track = token->getTrack();
    if (ptrack != track) {
        return false;
    }
    int beamstart1 = token->getValueInt("auto", "beamstart");
    int beamstart2 = ptoken->getValueInt("auto", "beamstart");
    if (beamstart1 == 0) {
        return false;
    }
    if (beamstart2 == 0) {
        return false;
    }

    std::vector<hum::HTp> data1 = getBeamNotes(token, beamstart1);
    std::vector<hum::HTp> data2 = getBeamNotes(ptoken, beamstart2);

    if (data1.size() != data2.size()) {
        return false;
    }
    if (data1.empty()) {
        return false;
    }

    for (int i = 0; i < (int)data1.size(); ++i) {
        hum::HumNum dur1 = data1[i]->getDuration();
        hum::HumNum dur2 = data2[i]->getDuration();
        if (dur1 != dur2) {
            return false;
        }
        if (data1[i]->isChord()) {
            return false;
        }
        if (data2[i]->isChord()) {
            return false;
        }
        int pitch1 = data1[i]->getBase40Pitch();
        int pitch2 = data2[i]->getBase40Pitch();
        if (pitch1 != pitch2) {
            return false;
        }
    }

    // The beam is a duplicate of the first layer beam, so
    // make sameas.
    for (int i = 0; i < (int)data1.size(); ++i) {
        data1[i]->setValue("auto", "suppress", 1);
    }
    std::string id = data2[0]->getValue("auto", "beamid");
    if ((id != "") && (id != "false")) {
        beam->SetSameas("#" + id);
    }

    return true;
}

//////////////////////////////
//
// HumdrumInput::getBeamNotes --
//

std::vector<hum::HTp> HumdrumInput::getBeamNotes(hum::HTp token, int beamstart)
{
    std::vector<hum::HTp> output;
    output.push_back(token);
    hum::HTp current = token->getNextToken();
    while (current) {
        if (current->isBarline()) {
            break;
        }
        if (!current->isData()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isNull()) {
            current = current->getNextToken();
            continue;
        }
        int beamend = current->getValueInt("auto", "beamend");
        output.push_back(current);
        if (beamend == beamstart) {
            break;
        }
        current = current->getNextToken();
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::handleGroupEnds --
//

void HumdrumInput::handleGroupEnds(
    const humaux::HumdrumBeamAndTuplet &tg, std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    if (tg.beamend && tg.tupletend) {
        if (tg.priority == 'T') {
            removeTuplet(elements, pointers);
            removeBeam(elements, pointers);
        }
        else {
            removeBeam(elements, pointers);
            removeTuplet(elements, pointers);
        }
    }
    else if (tg.beamend) {
        removeBeam(elements, pointers);
    }
    else if (tg.tupletend) {
        removeTuplet(elements, pointers);
    }

    if (tg.gbeamend) {
        // Grace note beams should not interact with
        // regular beams or tuplets.
        removeGBeam(elements, pointers);
    }
}

//////////////////////////////
//
// HumdrumInput::fillEmptyLayer --
//

void HumdrumInput::fillEmptyLayer(
    int staffindex, int layerindex, std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    std::vector<hum::HTp> &layerdata = m_layertokens[staffindex][layerindex];
    if ((layerdata.size() >= 2) && layerdata[0]->isBarline() && layerdata.back()->isBarline()) {
        hum::HumNum starttime = layerdata[0]->getDurationFromStart();
        hum::HumNum endtime = layerdata.back()->getDurationFromStart();
        hum::HumNum duration = endtime - starttime;
        addSpace(elements, pointers, duration, "straddle");
    }
}

//////////////////////////////
//
// HumdrumInput::checkForVerseLabels --
//

void HumdrumInput::checkForVerseLabels(hum::HTp token)
{
    if (!token) {
        return;
    }
    if (!token->isInterpretation()) {
        return;
    }
    std::vector<int> &rkern = m_rkern;
    int track = token->getTrack();
    int staffindex = rkern[track];
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    hum::HTp current = token->getNextFieldToken();
    while (current && (track == current->getTrack())) {
        current = current->getNextFieldToken();
    }
    while (current && !current->isStaff()) {
        if (!(current->isDataTypeLike("**text") || current->isDataTypeLike("**vdata"))) {
            current = current->getNextFieldToken();
            continue;
        }
        if (current->compare(0, 3, "*v:") == 0) {
            ss[staffindex].verse_labels.push_back(current);
        }
        else if (current->compare(0, 4, "*vv:") == 0) {
            ss[staffindex].verse_labels.push_back(current);
            ss[staffindex].verse_abbr_labels.push_back(current);
        }
        if (current->compare(0, 4, "*V:") == 0) {
            ss[staffindex].verse_abbr_labels.push_back(current);
        }
        current = current->getNextFieldToken();
    }
}

//////////////////////////////
//
// HumdrumInput::checkForLayerJoin --
//

bool HumdrumInput::checkForLayerJoin(int staffindex, int layerindex)
{
    return false; // layer@sameas is not yet allowed.
    /*
            if (!m_join) {
                    return false;
            }
            if (layerindex != 1) {
                    return false;
            }

       std::vector<std::vector<hum::HTp>>& layerdatas = m_layertokens[staffindex];
            std::vector<hum::HTp> data1;
            std::vector<hum::HTp> data2;
            for (int i=0; i<(int)layerdatas.at(0).size(); ++i) {
                    hum::HTp token = layerdatas.at(0).at(i);
                    if (token->isData()) {
                            data1.push_back(token);
                    } else if (*token == "*Xjoin") {
                            return false;
                    }
            }
            for (int i=0; i<(int)layerdatas.at(1).size(); ++i) {
                    hum::HTp token = layerdatas.at(1).at(i);
                    if (token->isData()) {
                            data2.push_back(token);
                    } else if (*token == "*Xjoin") {
                            return false;
                    }
            }
            if (data1.size() != data2.size()) {
                    return false;
            }
            for (int i=0; i<(int)data1.size(); ++i) {
                    if (data1[i]->isChord()) {
                            return false;
                    }
                    if (data2[1]->isChord()) {
                            return false;
                    }
                    hum::HumNum dur1 = data1[i]->getDuration();
                    hum::HumNum dur2 = data2[i]->getDuration();
                    if (dur1 != dur2) {
                            return false;
                    }
                    if (data1[i]->isRest() && data2[i]->isRest()) {
                            continue;
                    }
                    int pitch1 = data1[i]->getBase40Pitch();
                    int pitch2 = data2[i]->getBase40Pitch();
                    if (pitch1 != pitch2) {
                            continue;
                    }
            }

            // Two layers have same note/rest content so make the second layer
            // a sameas of the first layer:
       Layer *&layer = m_layer;
            std::string id = layer->GetID();
            hum::HumRegex hre;
            hre.replaceDestructive(id, "N1", "N\\d+");
            // m_layer->SetSameas("#" + id);

            return true;
    */
}

//////////////////////////////
//
// HumdrumInput::fillContentsOfLayer -- Fill the layer with musical data.
//

bool HumdrumInput::fillContentsOfLayer(int track, int startline, int endline, int layerindex)
{
    hum::HumdrumFile &infile = m_infiles[0];

    std::vector<hum::HumNum> &timesigdurs = m_timesigdurs;
    std::vector<int> &rkern = m_rkern;
    int staffindex = rkern[track];
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if (m_join) {
        bool status = checkForLayerJoin(staffindex, layerindex);
        if (status) {
            return true;
        }
    }

    if (staffindex < 0) {
        // not a kern spine.
        return false;
    }
    std::vector<hum::HTp> &layerdata = m_layertokens[staffindex][layerindex];

    Layer *&layer = m_layer;

    if (layerdata.size() == 0) {
        // nothing to do.
        return true;
    }

    if (!m_mens) {
        prepareInitialOttavas(layerdata[0]);
    }

    hum::HumNum starttime = 0;
    hum::HumNum endtime = 0;
    hum::HumNum duration = 0;
    if (!m_mens) {
        starttime = infile[startline].getDurationFromStart();
        endtime = infile[endline].getDurationFromStart() + infile[endline].getDuration();
        duration = endtime - starttime;
    }

    /* Why not allowed?
    if (timesigdurs[startline] != duration) {
        m_measure->SetMetcon(BOOLEAN_false);
    }
    */

    hum::HumNum layerstarttime = 0;
    hum::HumNum layerendtime = 0;
    if (!m_mens) {
        layerstarttime = infile[startline].getDurationFromStart();
        layerendtime = infile[endline].getDurationFromStart();
    }

    std::vector<hum::HumNum> prespace;
    getTimingInformation(prespace, layerdata, layerstarttime, layerendtime);

    if (emptyMeasures()) {
        if (timesigdurs[startline] == duration) {
            MRest *mrest = new MRest();
            m_layer->AddChild(mrest);
            // Assign a Humdrum ID here.
            hum::HTp trest = NULL;
            for (int i = 0; i < (int)layerdata.size(); ++i) {
                if (layerdata[i]->isRest()) {
                    trest = layerdata[i];
                    break;
                }
            }
            if (trest) {
                setLocationId(mrest, trest);
                if (m_humtype) {
                    embedQstampInClass(mrest, trest, *trest);
                }
            }
        }
        else {
            Rest *rest = new Rest();
            m_layer->AddChild(rest);
            setDuration(rest, duration);
        }
        return true;
    }

    std::vector<std::string> elements;
    std::vector<void *> pointers;
    elements.push_back("layer");
    pointers.push_back((void *)layer);

    if ((layerdata.size() == 2) && layerdata[0]->isBarline() && layerdata[1]->isBarline()) {
        fillEmptyLayer(staffindex, layerindex, elements, pointers);
        return true;
    }

    // Check for cases where there are only null interpretations in the measure
    // and insert a space in the measure (related to tied notes across barlines).
    if (layerOnlyContainsNullStuff(layerdata)) {
        fillEmptyLayer(staffindex, layerindex, elements, pointers);
        return true;
    }

    // If the layer contains only a single rest and the rest
    // is the same duration as the time signature, then
    // create a full measure rest (mrest).
    // Deal with time/clef/key changes in measures with
    // whole-measure rests later.  Also have to deal with
    // pedal mark in mrest meaures.
    if (hasFullMeasureRest(layerdata, timesigdurs[startline], duration)) {
        hum::HumNum meterDur;
        meterDur = 4;
        meterDur = meterDur / ss[staffindex].meter_bottom;
        meterDur = meterDur * ss[staffindex].meter_top;
        if (m_multirest[startline] > 1) {
            int tempendline = getMultiEndline(startline);
            MultiRest *multirest = new MultiRest();
            multirest->SetNum(m_multirest[startline]);
            appendElement(layer, multirest);
            for (int j = 0; j < (int)layerdata.size(); j++) {
                if (!layerdata[j]->isData()) {
                    continue;
                }
                processDirections(layerdata[j], staffindex);
            }
            setSystemMeasureStyle(startline, tempendline);
        }
        else {
            hum::HTp trest = NULL;
            for (int i = 0; i < (int)layerdata.size(); ++i) {
                if (!layerdata[i]->isData()) {
                    continue;
                }
                if (layerdata[i]->isRest()) {
                    trest = layerdata[i];
                    break;
                }
            }
            if (trest) {
                hum::HumNum duration = trest->getDuration();
                hum::HumNum mstartdur = layerdata[0]->getDurationFromStart();
                hum::HumNum menddur = layerdata.back()->getDurationFromStart();
                hum::HumNum mdur = menddur - mstartdur;
                hum::HumNum extradur = duration - mdur;
                if (extradur == 0) {
                    if (meterDur == duration) {
                        // duration of rest matches duration of meter so create mRest
                        MRest *mrest = new MRest();
                        appendElement(layer, mrest);
                        convertMRest(mrest, trest, -1, staffindex);
                    }
                    else {
                        // Duration of rest does not matches duration of meter so use a rest instead.
                        // This code currently cannot handle non-power-of-two rests such as
                        // a full measure rest of 5/8.
                        if (trest->find("yy") != std::string::npos) {
                            Space *irest = new Space();
                            if (m_doc->GetOptions()->m_humType.GetValue()) {
                                embedQstampInClass(irest, trest, *trest);
                            }
                            setLocationId(irest, trest);
                            appendElement(elements, pointers, irest);
                            convertRhythm(irest, trest);
                        }
                        else {
                            Rest *rest = new Rest();
                            convertRest(rest, trest, -1, staffindex);
                        }
                    }
                }
                else if (extradur > 0) {
                    // add a rest that is left justified in the measure
                    // (so not an mRest), and update the visual duration
                    // of the rest because there will be invisible rests
                    // added in later measure(s).
                    Rest *rest = new Rest;
                    appendElement(layer, rest);
                    convertRest(rest, trest, -1, staffindex);
                }
                else {
                    std::cerr << "Strange error for adding rest " << trest << std::endl;
                    std::cerr << "LINE: " << trest->getLineNumber() << ", FIELD: " << trest->getFieldNumber()
                              << std::endl;
                }
            }

            for (int z = 0; z < (int)layerdata.size(); ++z) {
                // Don't look at first item in full-measure rest if it is a barline.
                // This is to prevent text directions for previous measure being duplicated
                // at the end of the mesure.  Disallowing initial measure token should
                // be ok.
                if ((z == 0) && layerdata[z]->isBarline()) {
                    continue;
                }

                processDirections(layerdata[z], staffindex);

                if (layerdata[z]->isInterpretation()) {
                    handlePedalMark(layerdata[z]);
                }
            }
        }

        // Basic compensation for clef change (can be improved later):
        hum::HTp lastnote = NULL;
        for (int i = 0; i < (int)layerdata.size(); ++i) {
            if (layerdata[i]->isData() && !layerdata[i]->isNull()) {
                lastnote = layerdata[i];
            }
            if (!layerdata[i]->isClef()) {
                continue;
            }
            if (layerdata[i]->getDurationFromBarline() > 0) {
                // only insert a clef change after the whole-note rest
                // if the clef change is not really an initial clef.
                insertClefElement(elements, pointers, layerdata[i], lastnote);
            }
        }

        // check for fermatas to be placed on next barline
        if ((layerindex == 0) && (!layerdata.empty()) && (layerdata.back()->at(0) == '=')) {
            if (layerdata.back()->find(";") != std::string::npos) {
                addFermata(layerdata.back(), NULL);
            }
            if (layerdata.back()->find(",") != std::string::npos) {
                addBreath(layerdata.back(), NULL);
            }
        }

        // probably better to mark the rest in Humdrum data
        // as a full-measure rest here, and then process the
        // measure as normal below.
        return true;
    }

    std::vector<humaux::HumdrumBeamAndTuplet> tgs;
    prepareBeamAndTupletGroups(tgs, layerdata);
    fixLargeTuplets(tgs);

    if (m_debug) {
        printGroupInfo(tgs);
    }

    m_tupletscaling = 1;

    Note *note = NULL;

    // Used for splitting a rest across a clef:
    hum::HTp restSplitToken = NULL;
    hum::HumNum remainingSplitDur;

    hum::HumRegex hre;
    // processGlobalDirections(token, staffindex);

    hum::HTp lastnote = NULL;
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        hum::HTp token = layerdata[i];
        if (token->isData()) {
            lastnote = token;
        }
        if (prespace.at(i) > 0) {
            addSpace(elements, pointers, prespace.at(i), "straddle");
        }
        if (token->isData() && token->isNull()) {
            // print any global text directions attached to the null token
            // and then skip to next token.
            processDirections(token, staffindex);
            continue;
        }
        if (token->isInterpretation()) {
            processInterpretationStuff(token, staffindex);
            if (*token == "*join") {
                m_join = true;
                continue;
            }
            if (*token == "*Xjoin") {
                m_join = false;
                continue;
            }
            if ((staffindex == 0) && (token->compare(0, 8, "*tscale:") == 0)) {
                hum::HumRegex hree;
                if (hree.search(token, "^\\*tscale:(\\d+)/(\\d+)$")) {
                    int valuetop = hree.getMatchInt(1);
                    int valuebot = hree.getMatchInt(2);
                    if ((valuetop > 0) && (valuebot > 0)) {
                        hum::HumNum value = valuetop;
                        value /= valuebot;
                        m_localTempoScaling *= value;
                        Tempo *tempo = new Tempo();
                        tempo->SetMidiBpm(m_midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
                        setLocationId(tempo, token);
                        int staffindex = 0;
                        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
                        tempo->SetTstamp(tstamp.getFloat());
                        addChildMeasureOrSection(tempo);
                    }
                }
                else if (hree.search(token, "^\\*tscale:(\\d+)$")) {
                    hum::HumNum value = hree.getMatchInt(1);
                    if (value > 0) {
                        m_localTempoScaling *= value;
                        Tempo *tempo = new Tempo();
                        tempo->SetMidiBpm(m_midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
                        setLocationId(tempo, token);
                        int staffindex = 0;
                        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
                        tempo->SetTstamp(tstamp.getFloat());
                        addChildMeasureOrSection(tempo);
                    }
                }
            }

            if (ss[staffindex].verse) {
                checkForVerseLabels(token);
            }
            if (!m_mens) {
                handleOttavaMark(token, note);
                handleLigature(token);
                handleColoration(token);
                handleTempoChange(token);
                handlePedalMark(token);
                handleStaffStateVariables(token);
                handleStaffDynamStateVariables(token);
            }
            handleCustos(elements, pointers, layerdata, i);
            if (*token == "*rep") {
                int oldi = i;
                i = insertRepetitionElement(elements, pointers, layerdata, i);

                // Now go back and insert dynamics for the removed notes:
                for (int j = oldi; j <= i; j++) {
                    if (!layerdata[j]->isData()) {
                        continue;
                    }
                    processDynamics(layerdata[j], staffindex);
                    processDirections(layerdata[j], staffindex);
                }
            }
            if (hre.search(token, "^\\*color:(.*)")) {
                int ctrack = token->getTrack();
                int strack = token->getSubtrack();
                m_spine_color[ctrack][strack] = hre.getMatch(1);
                if (strack == 1) {
                    m_spine_color[ctrack][0] = m_spine_color[ctrack][1];
                }
                else if (strack == 0) {
                    for (int z = 1; z < (int)m_spine_color[ctrack].size(); z++) {
                        m_spine_color[ctrack][z] = m_spine_color[ctrack][0];
                    }
                }
            }

            bool forceClefChange = false;
            if (token->isClef() || (*token == "*")) {
                if (!(token->isMensLike() && token->getDurationFromStart() == 0)) {
                    if (token->getValueBool("auto", "clefChange")) {
                        forceClefChange = true;
                    }
                }
            }

            if (token->isMensLike()) {
                if (token->isClef()) {
                    if (ss.at(m_currentstaff - 1).last_clef != *token) {
                        if (forceClefChange) {
                            Clef *clef = insertClefElement(elements, pointers, token, lastnote);
                            setLocationId(clef, token);
                        }
                    }
                }
                if (m_mens) {
                    if (token->isMensurationSymbol()) {
                        // add mensuration change to layer.
                        setMensurationSymbol(m_layer, *token, staffindex, token);
                    }
                }
                else if (token->isMensurationSymbol() && (token->getDurationFromStart() > 0)) {
                    // add mensuration change to layer.
                    setMensurationSymbol(m_layer, *token, staffindex, token);
                }
            }
            else if (forceClefChange || (token->getDurationFromStart() != 0)) {
                if (token->isClef()) {
                    int subtrack = token->getSubtrack();
                    if (subtrack) {
                        subtrack--;
                    }

                    hum::HumNum durFromStart = token->getDurationFromStart();
                    hum::HumNum durFromBarline = token->getDurationFromBarline();

                    Clef *clef = NULL;
                    if ((durFromStart > 0) && (durFromBarline == 0)) {
                        // This clef will be inserted into a staffDef for the current
                        // staff, which is handled elsewhere.
                    }
                    else {
                        // Store in the layer as a cautionary staff.
                        clef = insertClefElement(elements, pointers, token, lastnote);
                    }

                    if (clef) {
                        if (token->find("yy") != std::string::npos) {
                            clef->SetVisible(BOOLEAN_false);
                        }
                        setLocationId(clef, token);
                        int diff = layerindex - subtrack;
                        if (diff > 0) {
                            std::string letter;
                            letter.push_back('a' + diff);
                            std::string id = clef->GetID();
                            id += letter;
                            clef->SetID(id);
                        }
                        if (restSplitToken != NULL) {
                            // Add the second part of a split invisible rest (or
                            // the invisible second part of a split visible rest):
                            Space *irest = new Space();
                            if (m_doc->GetOptions()->m_humType.GetValue()) {
                                embedQstampInClass(irest, restSplitToken, *restSplitToken);
                            }
                            setLocationId(irest, restSplitToken);
                            std::string id = irest->GetID();
                            id += "b";
                            irest->SetID(id);
                            appendElement(elements, pointers, irest);
                            // convertRhythm(irest, restSplitToken);
                            setRhythmFromDuration(irest, remainingSplitDur);
                            // processSlurs(restSplitToken);
                            // processPhrases(restSplitToken);
                            // processDynamics(restSplitToken, staffindex);
                            // processDirections(restSplitToken, staffindex);
                            // Store rest here to complete the split after the clef change.
                            restSplitToken = NULL;
                            remainingSplitDur = 0;
                        }
                    }
                }
                else if (token->isNull()) {
                    if ((i > 0) && (token->getLineIndex() == layerdata[i - 1]->getLineIndex())) {
                        // do nothing: duplicate layer clefs are handled elsewhere
                    }
                    else {
                        // duplicate clef changes in secondary layers
                        int xtrack = token->getTrack();
                        hum::HTp tok = token->getPreviousFieldToken();
                        while (tok) {
                            int ttrack = tok->getTrack();
                            if (ttrack == xtrack) {
                                if (tok->isClef()) {
                                    Clef *clef = insertClefElement(elements, pointers, tok, lastnote);
                                    setLocationId(clef, token);
                                    // Uncomment when clef->SetSameas() is available:
                                    // std::string sameas = "#clef-L";
                                    // sameas += to_string(tok->getLineNumber());
                                    // sameas += "F";
                                    // sameas += to_string(tok->getFieldNumber());
                                    // clef->SetSameas(sameas);
                                    break;
                                }
                            }
                            tok = tok->getPreviousFieldToken();
                        }
                    }
                }
            }
            if (token->isTimeSignature()) {
                // Now done at the measure level.  This location might
                // be good for time signatures which change in the
                // middle of measures.
                // insertMeterSigElement(elements, pointers, layerdata, i);
                processDirections(token, staffindex);
            }
            if ((*token == "*bar") || (token->compare(0, 5, "*bar:") == 0)) {
                BarLine *barline = new BarLine;
                setLocationId(barline, token);
                appendElement(elements, pointers, barline);
            }
            if (m_join && ((*token == "*a2") || (token->compare(0, 4, "*a2:") == 0)) && (token->getSubtrack() == 1)) {
                // Add "a 2" text
                Dir *dir = new Dir;
                addTextElement(dir, "a 2");
                setStaff(dir, m_currentstaff);
                setLocationId(dir, token); // adjust with new element class
                hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
                dir->SetTstamp(tstamp.getFloat());
                addChildBackMeasureOrSection(dir);
                setPlaceRelStaff(dir, "above");
            }
        }
        if (token->isBarline() && (!token->allSameBarlineStyle())) {
            // display a barline local to the staff
            if (i == 0) {
                // don't print a barline at the start of a measure (always?)
            }
            else {
                addBarLineElement(token, elements, pointers);
            }
        }
        if (!token->isData()) {
            continue;
        }

        if (token->isMensLike()) {
            convertMensuralToken(elements, pointers, token, staffindex);
            continue;
        }

        handleGroupStarts(tgs, elements, pointers, layerdata, i);

        if (token->getValueBool("auto", "tremoloBeam")) {
            if (token->find("L") == std::string::npos) {
                // ignore the ending note of a beamed group
                // of tremolos (a previous note in the tremolo
                // replaces display of this note).
                handleGroupEnds(tgs.at(i), elements, pointers);
                continue;
            }
        }

        if (token->getValueInt("auto", "suppress")) {
            // This element is not supposed to be printed,
            // probably due to being in a tremolo.

            // But first check for dynamics and text, which
            // should not be suppressed:
            processDynamics(token, staffindex);
            processDirections(token, staffindex);
            continue;
        }

        // conversion of **kern data to MEI:
        if (token->isChord()) {
            int chordnotecount = getChordNoteCount(token);
            if (chordnotecount < 1) {
                // invalid chord, so put a space in its place.
                if (m_signifiers.irest_color.empty() && m_signifiers.space_color.empty()) {
                    Space *irest = new Space();
                    if (m_doc->GetOptions()->m_humType.GetValue()) {
                        embedQstampInClass(irest, token, *token);
                    }
                    setLocationId(irest, token);
                    appendElement(elements, pointers, irest);
                    convertRhythm(irest, token);
                }
                else {
                    // force invisible rest to be displayed
                    Rest *rest = new Rest();
                    setLocationId(rest, token);
                    appendElement(elements, pointers, rest);
                    convertRest(rest, token, -1, staffindex);
                    int line = token->getLineIndex();
                    int field = token->getFieldIndex();
                    colorRest(rest, *token, line, field);
                    verticalRest(rest, *token);
                }
            }
            else {
                Chord *chord = new Chord();
                setLocationId(chord, token);

                if (m_hasTremolo && token->getValueBool("auto", "tremolo")) {
                    BTrem *btrem = new BTrem();
                    setBeamLocationId(btrem, tgs, layerdata, i);
                    // int slashes = token->getValueInt("auto", "slashes"); // MEI 3 method
                    int twodur = -(int)log2(hum::Convert::recipToDuration(token).getFloat());
                    switch (twodur) {
                        case 1: btrem->SetUnitdur(DURATION_8); break;
                        case 2: btrem->SetUnitdur(DURATION_16); break;
                        case 3: btrem->SetUnitdur(DURATION_32); break;
                        case 4: btrem->SetUnitdur(DURATION_64); break;
                        case 5: btrem->SetUnitdur(DURATION_128); break;
                    }
                    appendElement(btrem, chord);
                    appendElement(elements, pointers, btrem);
                }
                else if (m_hasTremolo && token->getValueBool("auto", "tremolo2")) {
                    FTrem *ftrem = new FTrem();
                    setBeamLocationId(ftrem, tgs, layerdata, i);
                    int beams = token->getValueInt("auto", "beams");
                    ftrem->SetBeams(beams);
                    int unit = token->getValueInt("auto", "unit");
                    switch (unit) {
                        case 8: ftrem->SetUnitdur(DURATION_8); break;
                        case 16: ftrem->SetUnitdur(DURATION_16); break;
                        case 32: ftrem->SetUnitdur(DURATION_32); break;
                        case 64: ftrem->SetUnitdur(DURATION_64); break;
                        case 128: ftrem->SetUnitdur(DURATION_128); break;
                    }
                    hum::HTp second = NULL;
                    for (int z = i + 1; z < (int)layerdata.size(); z++) {
                        if (layerdata[z]->getValueInt("auto", "tremoloAux")) {
                            second = layerdata[z];
                            break;
                        }
                    }
                    appendElement(ftrem, chord);
                    if (second) {
                        // ignoring slurs, ties, ornaments, articulations
                        if (second->isChord()) {
                            Chord *chord2 = new Chord();
                            setLocationId(chord2, second);
                            appendElement(ftrem, chord2);
                            convertChord(chord2, second, staffindex);
                        }
                        else {
                            Note *note2 = new Note();
                            setLocationId(note2, second);
                            appendElement(ftrem, note2);
                            convertNote(note2, second, 0, staffindex);
                        }
                        addSlur(ftrem, token, second);
                    }
                    appendElement(elements, pointers, ftrem);
                    addExplicitStemDirection(ftrem, token);
                }
                else {
                    appendElement(elements, pointers, chord);
                }

                elements.push_back("chord");
                pointers.push_back((void *)chord);
                processChordSignifiers(chord, token, staffindex);
                convertChord(chord, token, staffindex);
                checkForFingeredHarmonic(chord, token);
                popElementStack(elements, pointers);
                // maybe an extra pop here for tremolos?
                processSlurs(token);
                processPhrases(token);
                processDynamics(token, staffindex);
                assignAutomaticStem(chord, token, staffindex);
                addArticulations(chord, token);
                addOrnaments(chord, token);
                addArpeggio(chord, token);
                processDirections(token, staffindex);
            }
        }
        else if (token->isRest()) {
            if ((token->find("yy") != std::string::npos) && m_signifiers.irest_color.empty()
                && m_signifiers.space_color.empty()) {
                // Invisible rest (or note which should be invisible.
                if ((i < (int)layerdata.size() - 1) && layerdata[i + 1]->isClef()) {
                    hum::HumNum dur = token->getDuration();
                    hum::HumNum ndur = layerdata[i + 1]->getDurationFromStart() - token->getDurationFromStart();
                    hum::HumNum remainingDur = dur - ndur;

                    if ((ndur < dur) && isExpressibleDuration(ndur) && isExpressibleDuration(remainingDur)) {
                        // create a split invisible rest so that an intervening clef
                        // can be positioned properly.
                        // split the space into two pieces, this is the firsthalf.
                        Space *irest = new Space();
                        if (m_doc->GetOptions()->m_humType.GetValue()) {
                            embedQstampInClass(irest, token, *token);
                        }
                        setLocationId(irest, token);
                        appendElement(elements, pointers, irest);
                        // convertRhythm(irest, token);
                        setRhythmFromDuration(irest, ndur);
                        processSlurs(token);
                        processPhrases(token);
                        processDynamics(token, staffindex);
                        processDirections(token, staffindex);
                        // Store rest here to complete the split after the clef change.
                        restSplitToken = token;
                        remainingSplitDur = remainingDur;
                    }
                    else {
                        // normal space
                        Space *irest = new Space();
                        if (m_doc->GetOptions()->m_humType.GetValue()) {
                            embedQstampInClass(irest, token, *token);
                        }
                        setLocationId(irest, token);
                        appendElement(elements, pointers, irest);
                        convertRhythm(irest, token);
                        processSlurs(token);
                        processPhrases(token);
                        processDynamics(token, staffindex);
                        processDirections(token, staffindex);
                    }
                }
                else {
                    Space *irest = new Space();
                    if (m_doc->GetOptions()->m_humType.GetValue()) {
                        embedQstampInClass(irest, token, *token);
                    }
                    setLocationId(irest, token);
                    appendElement(elements, pointers, irest);
                    convertRhythm(irest, token);
                    processSlurs(token);
                    processPhrases(token);
                    processDynamics(token, staffindex);
                    processDirections(token, staffindex);
                }
            }
            else {
                int line = token->getLineIndex();
                int field = token->getFieldIndex();

                if ((i < (int)layerdata.size() - 1) && layerdata[i + 1]->isClef()) {
                    hum::HumNum dur = token->getDuration();
                    hum::HumNum ndur = layerdata[i + 1]->getDurationFromStart() - token->getDurationFromStart();
                    hum::HumNum remainingDur = dur - ndur;

                    if ((ndur < dur) && isExpressibleDuration(ndur) && isExpressibleDuration(remainingDur)) {
                        // Create a split rest so that an intervening clef
                        // can be positioned properly.  There will be two
                        // pieces: first: this original rest, with the visual
                        // duration unchanged, but with a gestural duration that
                        // is the "pre-clef" duration, and second: an invisible
                        // rest (space) that has the post-clef duration.
                        // Here we emit the original rest, and save off what
                        // the second rest should be.
                        Rest *rest1 = new Rest();
                        if (m_doc->GetOptions()->m_humType.GetValue()) {
                            embedQstampInClass(rest1, token, *token);
                        }
                        setLocationId(rest1, token);
                        appendElement(elements, pointers, rest1);
                        // convertRhythm(irest, token);
                        setVisualAndGesturalRhythmFromDuration(rest1, dur, ndur);
                        processSlurs(token);
                        processPhrases(token);
                        processDynamics(token, staffindex);
                        processDirections(token, staffindex);
                        // Store rest here to complete the split after the clef change.
                        restSplitToken = token;
                        remainingSplitDur = remainingDur;
                    }
                    else {
                        hum::HumNum restDur = hum::Convert::recipToDuration(token);
                        if ((restDur == duration) && (restDur == timesigdurs[startline])) {
                            // whole-measure rest with something else also in
                            // measure (such as grace notes).
                            MRest *mrest = new MRest();
                            setLocationId(mrest, token);
                            appendElement(elements, pointers, mrest);
                            // colorRest(mrest, *token, line, field);
                            verticalRest(mrest, *token);
                        }
                        else {
                            Rest *rest = new Rest();
                            setLocationId(rest, token);
                            appendElement(elements, pointers, rest);
                            convertRest(rest, token, -1, staffindex);
                            colorRest(rest, *token, line, field);
                            verticalRest(rest, *token);
                        }
                        processSlurs(token);
                        processPhrases(token);
                        processDynamics(token, staffindex);
                        processDirections(token, staffindex);
                    }
                }
                else {
                    hum::HumNum restDur = hum::Convert::recipToDuration(token);
                    if ((restDur == duration) && (restDur == timesigdurs[startline])) {
                        // whole-measure rest with something else also in
                        // measure (such as grace notes).
                        MRest *mrest = new MRest();
                        setLocationId(mrest, token);
                        appendElement(elements, pointers, mrest);
                        // colorRest(mrest, *token, line, field);
                        verticalRest(mrest, *token);
                    }
                    else {
                        Rest *rest = new Rest();
                        setLocationId(rest, token);
                        appendElement(elements, pointers, rest);
                        convertRest(rest, token, -1, staffindex);
                        colorRest(rest, *token, line, field);
                        verticalRest(rest, *token);
                    }
                    processSlurs(token);
                    processPhrases(token);
                    processDynamics(token, staffindex);
                    processDirections(token, staffindex);
                }
            }
        }
        else if (!token->isNote()) {

            // this is probably a **recip value without note or rest information
            // so print it as a space (invisible rest).
            if ((!m_signifiers.rspace_color.empty()) || (!m_signifiers.space_color.empty())) {
                // This should be invisible but someone wants it visible
                // and colored.
                Rest *rest = new Rest();
                setLocationId(rest, token);
                appendElement(elements, pointers, rest);
                convertRest(rest, token, -1, staffindex);
                processSlurs(token);
                processPhrases(token);
                processDynamics(token, staffindex);
                processDirections(token, staffindex);
                int line = token->getLineIndex();
                int field = token->getFieldIndex();
                colorRest(rest, *token, line, field);
            }
            else {
                Space *irest = new Space();
                if (m_doc->GetOptions()->m_humType.GetValue()) {
                    embedQstampInClass(irest, token, *token);
                }
                setLocationId(irest, token);
                appendElement(elements, pointers, irest);
                convertRhythm(irest, token);
                processSlurs(token);
                processPhrases(token);
                processDynamics(token, staffindex);
                processDirections(token, staffindex);
            }
        }
        else {
            // should be a note
            note = new Note();
            setStemLength(note, token);
            setLocationId(note, token);

            if (m_hasTremolo && token->getValueBool("auto", "tremolo")) {
                BTrem *btrem = new BTrem();
                setBeamLocationId(btrem, tgs, layerdata, i);
                // int slashes = token->getValueInt("auto", "slashes"); // MEI 3 method
                int twodur = -(int)log2(hum::Convert::recipToDuration(token).getFloat());
                switch (twodur) {
                    case 1: btrem->SetUnitdur(DURATION_8); break;
                    case 2: btrem->SetUnitdur(DURATION_16); break;
                    case 3: btrem->SetUnitdur(DURATION_32); break;
                    case 4: btrem->SetUnitdur(DURATION_64); break;
                    case 5: btrem->SetUnitdur(DURATION_128); break;
                }
                appendElement(btrem, note);
                appendElement(elements, pointers, btrem);
            }
            else if (m_hasTremolo && token->getValueBool("auto", "tremolo2")) {
                FTrem *ftrem = new FTrem();
                setBeamLocationId(ftrem, tgs, layerdata, i);
                int beams = token->getValueInt("auto", "beams");
                ftrem->SetBeams(beams);
                int unit = token->getValueInt("auto", "unit");
                switch (unit) {
                    case 8: ftrem->SetUnitdur(DURATION_8); break;
                    case 16: ftrem->SetUnitdur(DURATION_16); break;
                    case 32: ftrem->SetUnitdur(DURATION_32); break;
                    case 64: ftrem->SetUnitdur(DURATION_64); break;
                    case 128: ftrem->SetUnitdur(DURATION_128); break;
                }
                hum::HTp second = NULL;
                for (int z = i + 1; z < (int)layerdata.size(); z++) {
                    if (layerdata[z]->getValueInt("auto", "tremoloAux")) {
                        second = layerdata[z];
                        break;
                    }
                }
                appendElement(ftrem, note);
                addArticulations(note, token);
                if (second) {
                    // ignoring slurs, ties, ornaments, articulations
                    if (second->isChord()) {
                        Chord *chord2 = new Chord();
                        setLocationId(chord2, second);
                        appendElement(ftrem, chord2);
                        convertChord(chord2, second, staffindex);
                    }
                    else {
                        Note *note2 = new Note();
                        setLocationId(note2, second);
                        appendElement(ftrem, note2);
                        convertNote(note2, second, 0, staffindex);
                        addArticulations(note2, second);
                    }
                    addSlur(ftrem, token, second);
                }
                appendElement(elements, pointers, ftrem);
                addExplicitStemDirection(ftrem, token);
            }
            else {
                appendElement(elements, pointers, note);
            }

            convertNote(note, token, 0, staffindex);
            processSlurs(token);
            processPhrases(token);
            processDynamics(token, staffindex);
            assignAutomaticStem(note, token, staffindex);
            if (m_signifiers.nostem && token->find(m_signifiers.nostem) != std::string::npos) {
                note->SetStemVisible(BOOLEAN_false);
            }
            if (m_signifiers.hairpinAccent && token->find(m_signifiers.hairpinAccent) != std::string::npos) {
                addHairpinAccent(token);
            }
            if (m_signifiers.cuesize && token->find(m_signifiers.cuesize) != std::string::npos) {
                note->SetCue(BOOLEAN_true);
            }
            else if (m_staffstates.at(staffindex).cue_size.at(m_currentlayer)) {
                note->SetCue(BOOLEAN_true);
            }
            addArticulations(note, token);
            addOrnaments(note, token);
            addArpeggio(note, token);
            processDirections(token, staffindex);
        }

        handleGroupEnds(tgs.at(i), elements, pointers);
    }

    if (prespace.size() > layerdata.size()) {
        if (prespace.back() > 0) {
            // if there is empty space at the end of the layer.  The layer is
            // rhythmically too short, so add a space element to match the
            // amount of underfilling.
            addSpace(elements, pointers, prespace.back(), "filler");
        }
    }

    // check for fermatas to be placed on next barline
    if ((layerindex == 0) && (!layerdata.empty()) && (layerdata.back()->at(0) == '=')) {
        if (layerdata.back()->find(";") != std::string::npos) {
            addFermata(layerdata.back(), NULL);
        }
        if (layerdata.back()->find(",") != std::string::npos) {
            addBreath(layerdata.back(), NULL);
        }

        // check for rptend here, since the one for the last measure in
        // the music is missed by the inline processing.  But maybe limit
        // this one to only checking for the last measure.  Or move barline
        // styling here...
        if ((layerdata.back()->find(":|") != std::string::npos)
            || (layerdata.back()->find(":!") != std::string::npos)) {
            if (m_measure) {
                m_measure->SetRight(BARRENDITION_rptend);
            }
        }
    }

    // Check for repeat start at beginning of music.  The data for the very
    // first measure starts at the exclusive interpretation so that clefs
    // and time signatures and such are included.  If the first element
    // in the layer is an exclusive interpretation, then search for any
    // starting barline that should be checked for a repeat start:
    if (!layerdata.empty() && (layerdata[0]->compare(0, 2, "**") == 0)) {
        for (int i = 0; i < (int)layerdata.size(); ++i) {
            hum::HTp token = layerdata[i];
            if (token->isData()) {
                break;
            }
            if (!token->isBarline()) {
                continue;
            }
            if ((token->find("|:") != std::string::npos) || (token->find("!:") != std::string::npos)) {
                if (m_measure) {
                    m_measure->SetLeft(BARRENDITION_rptstart);
                }
            }
            break;
        }
    }

    // Check for repeat start at other places besides beginning of music:
    if ((layerindex == 0) && (!layerdata.empty()) && (layerdata[0]->at(0) == '=')) {
        if ((layerdata[0]->find("|:") != std::string::npos) || (layerdata[0]->find("!:") != std::string::npos)) {
            if (m_measure) {
                m_measure->SetLeft(BARRENDITION_rptstart);
            }
        }
    }

    if ((layerdata.size() > 0) && layerdata.back()->isBarline()) {
        processDirections(layerdata.back(), staffindex);
    }

    if (m_mens && (layerdata.size() > 0) && layerdata.back()->isBarline()) {
        // add barLine in mensural music
        addBarLineElement(layerdata.back(), elements, pointers);
    }

    return true;
}

//////////////////////////////
//
// HumdrumInput::isExpressibleDuration -- returns whether or not this
//      duration can be expressed as MEI @dur/@dots attributes. We limit
//      ourselves to three dots, and take into account the current tuplet
//      scaling.
//

bool HumdrumInput::isExpressibleDuration(hum::HumNum duration)
{
    // convert to whole note units
    hum::HumNum dur = duration / 4;
    // take into account current tuplet scaling
    dur *= m_tupletscaling;

    if (dur.getDenominator() == 1) {
        if (dur.getNumerator() == 2) {
            return true; // breve
        }
        else if (dur.getNumerator() == 3) {
            return true; // dotted breve
        }
        else if (dur.getNumerator() == 4) {
            return true; // long
        }
        else if (dur.getNumerator() == 6) {
            return true; // dotted long
        }
        else if (dur.getNumerator() == 8) {
            return true; // maxima
        }
        else if (dur.getNumerator() == 12) {
            return true; // dotted maxima
        }
    }

    // decide if the rhythm can be represented as 1/2**n with no dots.
    if (dur.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(dur.getDenominator())) {
            return true;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with one dot.
    hum::HumNum test1dot = (dur * 2) / 3;
    if (test1dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test1dot.getDenominator())) {
            return true;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with two dots.
    hum::HumNum test2dot = (dur * 4) / 7;
    if (test2dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test2dot.getDenominator())) {
            return true;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with three dots.
    hum::HumNum test3dot = (dur * 8) / 15;
    if (test3dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test3dot.getDenominator())) {
            return true;
        }
    }

    // duration required more than three dots or is not 1/2**n at all.
    return false;
}

data_DURATION HumdrumInput::oneOverDenominatorToDur(int denominator)
{
    switch (denominator) {
        case 1: return DURATION_1;
        case 2: return DURATION_2;
        case 4: return DURATION_4;
        case 8: return DURATION_8;
        case 16: return DURATION_16;
        case 32: return DURATION_32;
        case 64: return DURATION_64;
        case 128: return DURATION_128;
        case 256: return DURATION_256;
        case 512: return DURATION_512;
        case 1024: return DURATION_1024;
        case 2048: return DURATION_2048;
    }
    return DURATION_NONE;
}

//////////////////////////////
//
// HumdrumInput::isExpressibleDuration -- returns MEI @dur/@dots attributes
//      for a duration.
//

pair<data_DURATION, int> HumdrumInput::getDurAndDots(hum::HumNum duration)
{
    pair<data_DURATION, int> output;

    // convert to whole note units
    hum::HumNum dur = duration / 4;
    // take into account current tuplet scaling
    dur *= m_tupletscaling;

    if (dur.getDenominator() == 1) {
        if (dur.getNumerator() == 2) {
            output.first = DURATION_breve;
            output.second = 0;
            return output; // breve
        }
        else if (dur.getNumerator() == 3) {
            output.first = DURATION_breve;
            output.second = 1;
            return output; // dotted breve
        }
        else if (dur.getNumerator() == 4) {
            output.first = DURATION_long;
            output.second = 0;
            return output; // long
        }
        else if (dur.getNumerator() == 6) {
            output.first = DURATION_long;
            output.second = 1;
            return output; // dotted long
        }
        else if (dur.getNumerator() == 8) {
            output.first = DURATION_maxima;
            output.second = 0;
            return output; // maxima
        }
        else if (dur.getNumerator() == 12) {
            output.first = DURATION_maxima;
            output.second = 1;
            return output; // dotted maxima
        }
    }

    // decide if the rhythm can be represented as 1/2**n with no dots.
    if (dur.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(dur.getDenominator())) {
            output.first = oneOverDenominatorToDur(dur.getDenominator());
            output.second = 0;
            return output;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with one dot.
    hum::HumNum test1dot = (dur * 2) / 3;
    if (test1dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test1dot.getDenominator())) {
            output.first = oneOverDenominatorToDur(test1dot.getDenominator());
            output.second = 1;
            return output;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with two dots.
    hum::HumNum test2dot = (dur * 4) / 7;
    if (test2dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test2dot.getDenominator())) {
            output.first = oneOverDenominatorToDur(test2dot.getDenominator());
            output.second = 2;
            return output;
        }
    }

    // now decide if the rhythm can be represented as 1/2**n with three dots.
    hum::HumNum test3dot = (dur * 8) / 15;
    if (test3dot.getNumerator() == 1) {
        if (hum::Convert::isPowerOfTwo(test3dot.getDenominator())) {
            output.first = oneOverDenominatorToDur(test3dot.getDenominator());
            output.second = 3;
            return output;
        }
    }

    // duration required more than three dots or is not 1/2**n at all.
    output.first = DURATION_NONE;
    output.second = 0;
    return output;
}

//////////////////////////////
//
// HumdrumInput::processInterpretationStuff --
//

void HumdrumInput::processInterpretationStuff(hum::HTp token, int staffindex)
{
    if (!token->isInterpretation()) {
        return;
    }

    if (token->compare(0, 8, "*Xartic:") == 0) {
        if (token->find("simile") != std::string::npos) {
            std::string placement = "below";
            if (token->find(":a") != std::string::npos) {
                placement = "above";
            }
            bool bold = false;
            if (token->find(":B") != std::string::npos) {
                bold = true;
            }
            bool italic = true;
            int justification = 0;
            std::string color;
            int vgroup = -1;
            addDirection("simile", placement, bold, italic, token, staffindex, justification, color, vgroup);
            return;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForFingeredHarmonic --
//

void HumdrumInput::checkForFingeredHarmonic(Chord *chord, hum::HTp token)
{
    if (token->find("r") == std::string::npos) {
        // No fingered harmonic notes
        return;
    }
    hum::HumRegex hre;
    int scount = token->getSubtokenCount();
    if (scount != 3) {
        // only dealing with 3-note cases for now
        return;
    }
    std::vector<std::string> tstrings = token->getSubtokens();
    int zcount = 0;
    std::vector<pair<int, int>> pitches(scount);
    for (int i = 0; i < scount; ++i) {
        std::string mstring = tstrings[i];
        if (mstring.find("r") != std::string::npos) {
            hre.replaceDestructive(mstring, "", "r", "g");
            zcount++;
        }
        int base40 = hum::Convert::kernToBase40(mstring);
        pitches[i].first = i;
        pitches[i].second = base40;
    }
    if (zcount != 2) {
        // only dealing with fingered harmonic
        return;
    }
    sort(pitches.begin(), pitches.end(), [](pair<int, int> &a, pair<int, int> &b) { return a.second < b.second; });

    // The bottom two notes in the 3-note chord need to be labeled with "r"
    // and the top one should not have an "r".

    if (tstrings[pitches[0].first].find("r") == std::string::npos) {
        return;
    }
    if (tstrings[pitches[1].first].find("r") == std::string::npos) {
        return;
    }
    if (tstrings[pitches[2].first].find("r") != std::string::npos) {
        return;
    }

    // Indexes to notes in input token:
    int bottomi = pitches[0].first; // index of bottom note in input token
    int middlei = pitches[1].first; // index of middle note in input token
    int topi = pitches[2].first; // index of top note in input token

    std::vector<int> outputindex(3, -1);
    int counter = 0;
    if (tstrings[0].find("yy") == std::string::npos) {
        outputindex[0] = counter++;
    }
    if (tstrings[1].find("yy") == std::string::npos) {
        outputindex[1] = counter++;
    }
    if (tstrings[2].find("yy") == std::string::npos) {
        outputindex[2] = counter++;
    }

    // Indexes to notes in verovio chord (-1 means not present):
    int bottomo = outputindex[bottomi];
    int middleo = outputindex[middlei];
    int topo = outputindex[topi];

    // Find highest note present in output:
    int highesto = topo;
    if (highesto < 0) {
        highesto = middleo;
    }
    if (highesto < 0) {
        highesto = bottomo;
    }

    ArrayOfObjects &notes = chord->GetChildrenForModification();

    if (middleo >= 0) {
        Note *middle = vrv_cast<Note *>(notes.at(middleo));
        middle->SetHeadShape(HEADSHAPE_diamond);
    }

    // Mute all notes that are not the highest one in the chord:
    if (notes.size() > 1) {
        // mute all notes but the highest one in chord.
        if ((bottomo >= 0) && (bottomo != highesto)) {
            vrv_cast<Note *>(notes.at(bottomo))->SetVel(0);
        }
        if ((middleo >= 0) && (middleo != highesto)) {
            vrv_cast<Note *>(notes.at(middleo))->SetVel(0);
        }
        if ((topo >= 0) && (topo != highesto)) {
            vrv_cast<Note *>(notes.at(topo))->SetVel(0);
        }
    }

    // If the highest note is not the top note, then add .ges pitch
    // to the highest note.
    if ((highesto >= 0) && (highesto != topo)) {
        hum::HumPitch hpitch;
        hpitch.setKernPitch(tstrings.at(topi));
        int oct = hpitch.getOctave();
        vrv_cast<Note *>(notes.at(highesto))->SetOctGes(oct);
        switch (hpitch.getDiatonicPC()) {
            case 0: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_c); break;
            case 1: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_d); break;
            case 2: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_e); break;
            case 3: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_f); break;
            case 4: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_g); break;
            case 5: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_a); break;
            case 6: vrv_cast<Note *>(notes.at(highesto))->SetPnameGes(PITCHNAME_b); break;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::addBarLineElement --
//

void HumdrumInput::addBarLineElement(hum::HTp bartok, std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    if (bartok->find("-") != std::string::npos) {
        // probably do not want to have an invisible barline
        return;
    }

    BarLine *barline = new BarLine();
    setLocationId(barline, bartok);

    if (bartok->compare(0, 2, "==") == 0) {
        barline->SetForm(BARRENDITION_end);
    }
    else if (bartok->find(":|!|:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptboth);
    }
    else if (bartok->find(":!!:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptboth);
    }
    else if (bartok->find(":||:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptboth);
    }
    else if (bartok->find(":!:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptboth);
    }
    else if (bartok->find(":|:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptboth);
    }
    else if (bartok->find(":|") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptend);
    }
    else if (bartok->find(":!") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptend);
    }
    else if (bartok->find("!:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptstart);
    }
    else if (bartok->find("|:") != std::string::npos) {
        barline->SetForm(BARRENDITION_rptstart);
    }
    else if (bartok->find("||") != std::string::npos) {
        barline->SetForm(BARRENDITION_dbl);
    }
    else if (bartok->find("-") != std::string::npos) {
        barline->SetForm(BARRENDITION_invis);
    }
    else if (bartok->find("..") != std::string::npos) {
        barline->SetForm(BARRENDITION_dbldotted);
    }
    else if (bartok->find(".") != std::string::npos) {
        barline->SetForm(BARRENDITION_dotted);
    }
    else if (bartok->find("::") != std::string::npos) {
        barline->SetForm(BARRENDITION_dbldashed);
    }
    else if (bartok->find(":") != std::string::npos) {
        barline->SetForm(BARRENDITION_dashed);
    }
    else {
        barline->SetForm(BARRENDITION_single);
    }

    appendElement(elements, pointers, barline);
}

//////////////////////////////
//
// HumdrumInput::layerOnlyContainsNullStuff -- layerdata only contains
//   barlines (at the endpoints) and null tokens ".", "!", or "*".
//

bool HumdrumInput::layerOnlyContainsNullStuff(std::vector<hum::HTp> &data)
{
    for (int i = 0; i < (int)data.size(); ++i) {
        if (data[i]->isBarline()) {
            continue;
        }
        if (!data[i]->isNull()) {
            return false;
        }
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::addHairpinAccent -- Simulate a hairpin accent with <> text dir.
//

void HumdrumInput::addHairpinAccent(hum::HTp token)
{
    auto pos = token->find(m_signifiers.hairpinAccent);
    if (pos == std::string::npos) {
        return;
    }
    int position = -1; // place below by default (may change based on layer position later).
    bool setpos = false;
    if ((int)pos < (int)token->size() - 1) {
        if (m_signifiers.above == token->at(pos + 1)) {
            position = +1;
            setpos = true;
        }
    }

    int track = token->getTrack();
    std::vector<int> &rkern = m_rkern;
    int staffindex = rkern[track];

    Dir *dir = new Dir();
    setStaff(dir, m_currentstaff);
    setLocationId(dir, token); // adjust with new element class
    hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    dir->SetTstamp(tstamp.getFloat());

    if (position > 0) {
        setPlaceRelStaff(dir, "above", setpos);
        addChildBackMeasureOrSection(dir);
    }
    else if (position < 0) {
        setPlaceRelStaff(dir, "below", setpos);
        addChildBackMeasureOrSection(dir);
    }
    else {
        addChildBackMeasureOrSection(dir);
    }

    Rend *rend = new Rend();
    dir->AddChild(rend);
    addTextElement(rend, "<>");
}

//////////////////////////////
//
// HumdrumInput::getMultiEndline -- Return the ending barline index of a multibar rest.
//

int HumdrumInput::getMultiEndline(int startindex)
{
    int found = -1;
    int index = startindex;
    for (int i = index; i < (int)m_multirest.size(); ++i) {
        if (m_multirest[i] == -1) {
            found = i;
            break;
        }
    }
    if (found < 0) {
        return startindex;
    }
    int found2 = -1;
    for (int i = found; i < (int)m_multirest.size(); ++i) {
        if (m_multirest[i] != -1) {
            found2 = i;
            break;
        }
    }
    if (found2 < 0) {
        return found;
    }
    return found2;
}

//////////////////////////////
//
// HumdrumInput::addSlur -- Check if there is a slur start and
//   end at the start/end of the tremolo group.
//

void HumdrumInput::addSlur(FTrem *ftrem, hum::HTp start, hum::HTp ending)
{
    if (ending->find(')') == std::string::npos) {
        // no slur ending
        return;
    }
    if (ending->find('J') == std::string::npos) {
        // no beam end (there could be weird unbeamed cases perhaps)
        return;
    }

    if (start->find('(') == std::string::npos) {
        // no slur start on fTrem, but there is a slur end from somewhere else
        processSlurs(ending);
        return;
    }
    if (start->find('L') == std::string::npos) {
        // no beam start (there could be weird unbeamed cases perhaps)
        return;
    }

    std::string firstid = ftrem->GetChild(0)->GetID();
    std::string secondid = ftrem->GetChild(1)->GetID();

    // should also deal with chord notes in ID.
    int endline = ending->getLineNumber();
    int endfield = ending->getFieldNumber();
    std::string lastid = "";
    lastid += "-L" + to_string(endline);
    lastid += "F" + to_string(endfield);

    std::string slurid = firstid;
    slurid += lastid;
    hum::HumRegex hre;
    hre.replaceDestructive(slurid, "slur", "^note");

    // maybe a problem if not all of the slurs on ending token
    // are ftrem (may result in multiple slurs for non-ftren slurs.
    processSlurs(ending);
    // appendTypeTag(slur, "ftrem");
}

//////////////////////////////
//
// addExplicitStemDirection -- Check if there is an explicit direction for
//   the FTrem element.  This can be either an above/below signifier
//   after the beam on the first token of the ftrem group, or it can be
//   the stem direction on the first note of the tremolo group.
//

void HumdrumInput::addExplicitStemDirection(FTrem *ftrem, hum::HTp start)
{
    int direction = 0;
    int showplace = false;
    if (start->find('/') != std::string::npos) {
        direction = +1;
    }
    else if (start->find('\\') != std::string::npos) {
        direction = -1;
    }
    else {
        hum::HumRegex hre;
        if (m_signifiers.above) {
            std::string value = "[LJkK]+";
            value += m_signifiers.above;
            if (hre.search(start, value)) {
                direction = +1;
                showplace = true;
            }
        }
        else if (m_signifiers.below) {
            std::string value = "[LJkK]+";
            value += m_signifiers.below;
            if (hre.search(start, value)) {
                direction = -1;
                showplace = true;
            }
        }
    }

    if (direction == 0) {
        return;
    }

    int count = ftrem->GetChildCount();

    // also deal with chords later
    for (int i = 0; i < count; ++i) {
        Object *obj = ftrem->GetChild(i);
        if (obj->GetClassName() == "Note") {
            if (direction > 0) {
                ((Note *)obj)->SetStemDir(STEMDIRECTION_up);
                if (m_humtype && showplace) {
                    appendTypeTag((Note *)obj, "placed");
                }
            }
            else {
                ((Note *)obj)->SetStemDir(STEMDIRECTION_down);
                if (m_humtype && showplace) {
                    appendTypeTag((Note *)obj, "placed");
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::handleTempoChange -- Generate <tempo> from *MM# interpretation as
//    long as there is no <tempo> text that will use the tempo *MM# as @midi.bpm.
//    *MM at the start of the music is ignored (placed separately into scoreDef).
//

void HumdrumInput::handleTempoChange(hum::HTp token)
{
    if (!token->isInterpretation()) {
        return;
    }
    hum::HumRegex hre;
    if (!hre.search(token, "^\\*MM(\\d+\\.?\\d*)")) {
        return;
    }
    hum::HumNum ttime = token->getDurationFromStart();
    if (ttime == 0) {
        // ignore starting tempo setting since it is handled
        // by scoreDef.
        return;
    }

    double midibpm = int(hre.getMatchDouble(1) + 0.5);
    if (midibpm <= 0) {
        return;
    }
    m_midibpm = midibpm;

    bool nearOmd = isNearOmd(token);
    if (nearOmd) {
        return;
    }

    bool hastempo = hasTempoTextAfter(token);
    if (hastempo) {
        return;
    }

    // only insert the tempo if there is no higher staff
    // that has a tempo marking at the same time
    bool islast = isLastStaffTempo(token);
    if (!islast) {
        return;
    }

    Tempo *tempo = new Tempo();
    tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
    setLocationId(tempo, token);
    int staffindex = 0;
    hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    tempo->SetTstamp(tstamp.getFloat());
    addChildMeasureOrSection(tempo);
}

//////////////////////////////
//
// HumdrumInput::isLastStaffTempo --
//

bool HumdrumInput::isLastStaffTempo(hum::HTp token)
{
    int field = token->getFieldIndex() + 1;
    int track = token->getTrack();
    hum::HumdrumLine &line = *(token->getOwner());
    for (int i = field; i < line.getFieldCount(); ++i) {
        hum::HTp newtok = line.token(i);
        int newtrack = newtok->getTrack();
        if (track == newtrack) {
            continue;
        }
        if (!newtok->isStaff()) {
            continue;
        }
        if (newtok->compare(0, 3, "*MM") == 0) {
            return false;
        }
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::hasTempoTextAfter -- Used to check of *MM# tempo change has potential <tempo>
//    text after it, but before any data.  Will not cross a measure boundary.
//    Input token is assumed to be a *MM interpertation (MIDI-like tempo change)
//    Algorithm: Find the first note after the input token and then check for a local
//    or global LO:TX parameter that applies to that note (for local LO:TX).
//

bool HumdrumInput::hasTempoTextAfter(hum::HTp token)
{
    hum::HumdrumFile &infile = *(token->getOwner()->getOwner());
    int startline = token->getLineIndex();
    hum::HTp current = token->getNextToken();
    if (!current) {
        return false;
    }

    // search for local LO:TX:
    while (current && !current->isData()) {
        current = current->getNextToken();
    }
    if (!current) {
        // No more data: at the end of the music.
        return false;
    }
    hum::HTp data = current;
    int dataline = data->getLineIndex();
    // now work backwards through all null local comments and !LO: parameters searching
    // for potential tempo text
    std::vector<hum::HTp> texts;
    current = data->getPreviousToken();
    int line = current->getLineIndex();
    if (!current) {
        return false;
    }
    while (current && (line > startline)) {
        if (!current->isLocalComment()) {
            break;
        }
        if (current->compare(0, 7, "!LO:TX:") == 0) {
            texts.push_back(current);
        }
        current = current->getPreviousToken();
        line = current->getLineIndex();
    }
    for (int i = 0; i < (int)texts.size(); ++i) {
        bool status = isTempoishText(texts[i]);
        if (status) {
            return true;
        }
    }

    // now check for global tempo text;
    texts.clear();
    for (int i = dataline - 1; i > startline; --i) {
        hum::HTp gtok = infile.token(i, 0);
        if (gtok->compare(0, 8, "!!LO:TX:") == 0) {
            texts.push_back(gtok);
        }
    }
    for (int i = 0; i < (int)texts.size(); ++i) {
        bool status = isTempoishText(texts[i]);
        if (status) {
            return true;
        }
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::isTempoishText -- Return true if the text is probably tempo indication.
//

bool HumdrumInput::isTempoishText(hum::HTp token)
{
    hum::HumRegex hre;
    if (hre.search(token, ":tempo:")) {
        return true;
    }
    if (hre.search(token, ":tempo$")) {
        return true;
    }
    if (!hre.search(token, ":t=([^:]+)")) {
        return false;
    }
    std::string text = hre.getMatch(1);
    if (hre.search(text, "\\[.*?\\]\\s*=.*\\d\\d")) {
        return true;
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::isNearOmd -- Returns true of the line of the token is adjacent to
//    An OMD line, with the boundary being a data line (measures are included).
//

bool HumdrumInput::isNearOmd(hum::HTp token)
{
    int tline = token->getLineIndex();
    hum::HumdrumFile &infile = *(token->getOwner()->getOwner());

    for (int i = tline - 1; tline >= 0; --i) {
        hum::HTp ltok = infile.token(i, 0);
        if (ltok->isData()) {
            break;
        }
        if (!infile[i].isReference()) {
            continue;
        }
        if (ltok->compare(0, 6, "!!!OMD") == 0) {
            return true;
        }
    }

    for (int i = tline + 1; tline < infile.getLineCount(); ++tline) {
        hum::HTp ltok = infile.token(i, 0);
        if (ltok->isData()) {
            break;
        }
        if (!infile[i].isReference()) {
            continue;
        }
        if (ltok->compare(0, 6, "!!!OMD") == 0) {
            return true;
        }
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::handleLigature --
//

void HumdrumInput::handleLigature(hum::HTp token)
{
    if (token->compare("*lig") != 0) {
        return;
    }

    // find end interpretation of ligature
    hum::HTp ligend = token->getNextToken();
    hum::HTp firstnote = NULL;
    hum::HTp lastnote = NULL;
    while (ligend) {
        if (ligend->compare("*Xlig") == 0) {
            break;
        }
        if (ligend->isNote()) {
            if (!firstnote) {
                firstnote = ligend;
            }
            lastnote = ligend;
        }
        ligend = ligend->getNextToken();
    }
    if (!ligend) {
        // could not find a matching ending for the
        // ligature start.  For now ignore it.  Perhaps
        // later a partial ligature bracket could be
        // rendered (but more likely a data error).
        return;
    }
    if (!firstnote) {
        // strange case: no notes in ligature
        return;
    }
    if (!lastnote) {
        // strange case: no notes in ligature
        return;
    }

    BracketSpan *ligature = new BracketSpan();

    int startline = token->getLineNumber();
    int startfield = token->getFieldNumber();
    int stopline = ligend->getLineNumber();
    int stopfield = ligend->getFieldNumber();
    std::string id = "ligature";
    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);
    id += "-L" + to_string(stopline);
    id += "F" + to_string(stopfield);
    ligature->SetID(id);

    // not considering if notes are in chords (which they should not)
    std::string startid = getLocationId("note", firstnote);
    ligature->SetStartid("#" + startid);
    std::string endid = getLocationId("note", lastnote);
    ligature->SetEndid("#" + endid);

    ligature->SetLform(LINEFORM_solid);
    ligature->SetFunc("ligature");

    addChildMeasureOrSection(ligature);
}

//////////////////////////////
//
// HumdrumInput::handleColoration --
//

void HumdrumInput::handleColoration(hum::HTp token)
{
    if (token->compare("*col") != 0) {
        return;
    }

    // find end interpretation of coloration
    hum::HTp colend = token->getNextToken();
    hum::HTp firstnote = NULL;
    hum::HTp lastnote = NULL;
    while (colend) {
        if (colend->compare("*Xcol") == 0) {
            break;
        }
        if (colend->isNote() || colend->isRest()) {
            if (!firstnote) {
                firstnote = colend;
            }
            lastnote = colend;
        }
        colend = colend->getNextToken();
    }
    if (!colend) {
        // could not find a matching ending for the
        // coloration start.  For now ignore it.  Perhaps
        // later a partial coloration bracket could be
        // rendered (but more likely a data error).
        return;
    }
    if (!firstnote) {
        // strange case: no notes in coloration
        return;
    }
    if (!lastnote) {
        // strange case: no notes in coloration
        return;
    }

    BracketSpan *coloration = new BracketSpan();

    int startline = token->getLineNumber();
    int startfield = token->getFieldNumber();
    int stopline = colend->getLineNumber();
    int stopfield = colend->getFieldNumber();
    std::string id = "coloration";
    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);
    id += "-L" + to_string(stopline);
    id += "F" + to_string(stopfield);
    coloration->SetID(id);

    // not considering if notes are in chords (which they should not)
    std::string startid;
    if (firstnote->isNote()) {
        startid = getLocationId("note", firstnote);
    }
    else {
        startid = getLocationId("rest", firstnote);
    }
    coloration->SetStartid("#" + startid);

    std::string endid;
    if (lastnote->isNote()) {
        endid = getLocationId("note", lastnote);
    }
    else {
        endid = getLocationId("rest", lastnote);
    }
    coloration->SetEndid("#" + endid);

    // data_LINEWIDTH lw;
    // lw.SetLineWidthTerm(LINEWIDTHTERM_medium);
    // coloration->SetLwidth(lw);
    coloration->SetFunc("coloration");

    addChildMeasureOrSection(coloration);
}

//////////////////////////////
//
// HumdrumInput::assignAutomaticStem --
//   X = no automatic stem assignment
//   x = no no stem
//   \ = stem down
//   / = stem up
//

template <class ELEMENT> void HumdrumInput::assignAutomaticStem(ELEMENT element, hum::HTp tok, int staffindex)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    char value = ss.at(staffindex).stem_type.at(m_currentlayer);
    if (value != 'X') {
        char hasstem = tok->hasStemDirection();
        if (!hasstem) {
            switch (value) {
                case '/': element->SetStemDir(STEMDIRECTION_up); break; // force stem up
                case '\\': element->SetStemDir(STEMDIRECTION_down); break; // force stem down
                case 'x': element->SetStemVisible(BOOLEAN_false); break; // force no stem
            }
        }
    }
    bool visible = ss.at(staffindex).stem_visible.at(m_currentlayer);
    if (!visible) {
        element->SetStemVisible(BOOLEAN_false);
    }
}

//////////////////////////////
//
// HumdrumInput::prepareInitialOttavas -- Check for an ottava mark start.
//
//

void HumdrumInput::prepareInitialOttavas(hum::HTp token)
{
    if (!token) {
        return;
    }
    hum::HumNum starttime = token->getDurationFromStart();
    if (starttime > 0) {
        return;
    }
    int subtrack = token->getSubtrack();
    if (subtrack > 1) {
        // only check for ottavas in the first layer.
        return;
    }

    hum::HTp tok = token->getPreviousToken();
    while (tok) {
        if (!tok->isInterpretation()) {
            tok = tok->getPreviousToken();
        }
        if (*tok == "*8va") {
            handleOttavaMark(tok, NULL);
            break;
        }
        else if (*tok == "*8ba") {
            handleOttavaMark(tok, NULL);
            break;
        }
        else if (*tok == "*15ma") {
            handleOttavaMark(tok, NULL);
            break;
        }
        else if (*tok == "*15ba") {
            handleOttavaMark(tok, NULL);
            break;
        }
        tok = tok->getPreviousToken();
    }
}

/////////////////////////////
//
// HumdrumInput::popElementStack --
//

void HumdrumInput::popElementStack(std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    elements.pop_back();
    pointers.pop_back();
}

//////////////////////////////
//
// HumdrumInput::convertMensuralToken --  Convert a mensural token to MEI.
//     Not dealing with chords yet.
//

void HumdrumInput::convertMensuralToken(
    std::vector<std::string> &elements, std::vector<void *> &pointers, hum::HTp token, int staffindex)
{
    if (token->isNull()) {
        return;
    }
    if (!token->isMensLike()) {
        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    bool roff = token->hasRectaLigatureEnd();
    bool ooff = token->hasObliquaLigatureEnd();
    bool ron = token->hasRectaLigatureBegin();
    bool oon = token->hasObliquaLigatureBegin();
    bool turnoffrecta = false;
    bool embeddedobliqua = false;

    if (roff) {
        turnoffrecta = true;
    }
    if (ooff) {
        if ((elements.back() == "ligature") && !ss[staffindex].ligature_obliqua) {
            embeddedobliqua = true;
        }
    }
    if (ooff && ss[staffindex].ligature_recta) {
        embeddedobliqua = true;
    }

    if (oon && (elements.back() == "ligature")) {
        embeddedobliqua = true;
    }

    if (token->hasLigatureBegin()) {
        if (ron) {
            ss[staffindex].ligature_recta = true;
        }
        if (ron && oon) {
            ss[staffindex].ligature_recta = true;
            ss[staffindex].ligature_obliqua = true;
            // if both at same time, then assume obliqua is the start
            // of a compound ligature
            Ligature *ligature = new Ligature();
            std::string id = getLocationId("ligature", token);
            ligature->SetID(id);
            ligature->SetForm(LIGATUREFORM_recta);
            appendElement(elements, pointers, ligature);
            elements.push_back("ligature");
            pointers.push_back((void *)ligature);
            embeddedobliqua = true;
        }
        else if (oon && elements.back() == "ligature") {
            // if already in a ligature, encode obliqua differently
            // this state variable will set note@lig="obliqua" further below
            ss[staffindex].ligature_obliqua = true;
        }
        else if (oon) {
            // create a new obliqua ligature
            ss[staffindex].ligature_obliqua = true;
            Ligature *ligature = new Ligature();
            std::string id = getLocationId("ligature", token);
            ligature->SetID(id);
            ligature->SetForm(LIGATUREFORM_obliqua);
            appendElement(elements, pointers, ligature);
            elements.push_back("ligature");
            pointers.push_back((void *)ligature);
        }
        else {
            // create a new recta ligature (which could be compoound and
            // contain an obliqua, which will be handled above.
            Ligature *ligature = new Ligature();
            std::string id = getLocationId("ligature", token);
            ligature->SetID(id);
            ligature->SetForm(LIGATUREFORM_recta);
            appendElement(elements, pointers, ligature);
            elements.push_back("ligature");
            pointers.push_back((void *)ligature);
            ss[staffindex].ligature_recta = true;
        }
    }

    if (token->isRest()) {
        Rest *rest = new Rest();
        setLocationId(rest, token);
        appendElement(elements, pointers, rest);
        convertRest(rest, token, -1, staffindex);
        if (token->find("~") != std::string::npos) {
            // rest->SetColored(BOOLEAN_true);
            if (ss.at(staffindex).mensuration_type == 1) {
                // black notation so add red coloring of colored rest
                rest->SetColor("red");
            }
        }
    }
    else if (token->isNote()) {
        Note *note = new Note();
        setLocationId(note, token);
        if (embeddedobliqua) {
            note->SetLig(LIGATUREFORM_obliqua);
        }
        appendElement(elements, pointers, note);
        convertNote(note, token, 0, staffindex);
        if (token->find("~") != std::string::npos) {
            note->SetColored(BOOLEAN_true);
            if (ss.at(staffindex).mensuration_type == 1) {
                // black notation so add red coloring of colored note
                note->SetColor("red");
            }
        }
        if (token->find("k") != std::string::npos) {
            addPlicaUp(note);
        }
        if (token->find("K") != std::string::npos) {
            addPlicaDown(note);
        }
        processSlurs(token);
        processPhrases(token);
        processDirections(token, staffindex);
        bool hasstem = false;
        std::string text = *token;
        for (int i = 0; i < (int)text.size(); ++i) {
            switch (text[i]) {
                case 'M': hasstem = true; break;
                case 'm': hasstem = true; break;
                case 'U': hasstem = true; break;
                case 'u': hasstem = true; break;
                case 'L': hasstem = true; break;
                case 'X': hasstem = true; break;
            }
            if (hasstem) {
                break;
            }
        }
        if (hasstem) {
            assignAutomaticStem(note, token, staffindex);
        }

        if (token->find(':') != std::string::npos) {
            Dot *dot = new Dot();
            appendElement(elements, pointers, dot);
        }
    }

    if (roff || ooff) {
        if (elements.back() == "ligature") {
            if (roff) {
                popElementStack(elements, pointers);
            }
            else if (ooff && !turnoffrecta && !ss[staffindex].ligature_recta) {
                popElementStack(elements, pointers);
            }
        }
        else {
            std::cerr << "WARNING: unmatched ligature ending" << std::endl;
        }
    }
    if (roff) {
        ss[staffindex].ligature_recta = false;
    }
    if (ooff) {
        ss[staffindex].ligature_obliqua = false;
    }
}

//////////////////////////////
//
// HumdrumInput::addPlicaDown --
//

void HumdrumInput::addPlicaDown(Note *note)
{
    Plica *plica = new Plica();
    plica->SetDir(STEMDIRECTION_basic_down);
    note->AddChild(plica);
}

//////////////////////////////
//
// HumdrumInput::addPlicaUp --
//

void HumdrumInput::addPlicaUp(Note *note)
{
    Plica *plica = new Plica();
    plica->SetDir(STEMDIRECTION_basic_up);
    note->AddChild(plica);
}

//////////////////////////////
//
// HumdrumInput::addArticulations --
//
// from: libmei/atttypes.h:
// enum data_ARTICULATION {
//    ARTICULATION_NONE = 0,             implemented:
//    ARTICULATION_acc,                        *
//    ARTICULATION_stacc,                      *
//    ARTICULATION_ten,                        *
//    ARTICULATION_stacciss,                   *
//    ARTICULATION_marc,                       *
//    ARTICULATION_marc_stacc,                 * or use "marc stacc"
//    ARTICULATION_spicc,
//    ARTICULATION_doit,
//    ARTICULATION_scoop,
//    ARTICULATION_rip,
//    ARTICULATION_plop,
//    ARTICULATION_fall,
//    ARTICULATION_longfall,
//    ARTICULATION_bend,
//    ARTICULATION_flip,
//    ARTICULATION_smear,
//    ARTICULATION_shake,
//    ARTICULATION_dnbow,
//    ARTICULATION_upbow,
//    ARTICULATION_harm,                       *
//    ARTICULATION_snap,
//    ARTICULATION_fingernail,
//    ARTICULATION_ten_stacc,                  * or use "ten stacc"
//    ARTICULATION_damp,
//    ARTICULATION_dampall,
//    ARTICULATION_open,
//    ARTICULATION_stop,
//    ARTICULATION_dbltongue,
//    ARTICULATION_trpltongue,
//    ARTICULATION_heel,
//    ARTICULATION_toe,
//    ARTICULATION_tap,
//    ARTICULATION_lhpizz,
//    ARTICULATION_dot,
//    ARTICULATION_stroke
//};
//

template <class ELEMENT> void HumdrumInput::addArticulations(ELEMENT element, hum::HTp token)
{

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_rkern[token->getTrack()];

    // store artics in random access grid, along with their staff positions:
    std::vector<int> articcount(128, 0); // number of explicit articulations (when duplicating with "X" postfix).
    std::vector<int> articloc(128, 0);
    std::vector<int> articpos(128, 0);
    std::vector<bool> showpos(128, 0);
    std::vector<int> articges(128, 0); // is it a gestural articulation?
    bool textTenuto = false;
    bool textTenutoBelow = false;
    char ch;
    char nextch;
    char posch;
    char pos2ch;
    char pos3ch;
    std::string nohidden = *token;
    if (nohidden.find("yy") != std::string::npos) {
        std::vector<std::string> tstrings = token->getSubtokens();
        nohidden = "";
        int counter = 0;
        for (int i = 0; i < (int)tstrings.size(); ++i) {
            if (tstrings[i].find("yy") != std::string::npos) {
                continue;
            }
            if (counter) {
                nohidden += " ";
                counter++;
            }
            nohidden += tstrings[i];
        }
    }
    int tsize = (int)nohidden.size();
    for (int i = 0; i < tsize; ++i) {
        ch = nohidden.at(i);
        if (i < (int)nohidden.size() - 1) {
            nextch = nohidden.at(i + 1);
        }
        else {
            nextch = '\0';
        }
        if ((ch == 'o') && (nextch == 'y')) {
            // Don't show harmonics on chords where the
            // harmonic is on an invisible note.
            continue;
        }
        int intch = (unsigned char)ch;
        if (intch < 0) {
            // ignore UTF-8 characters
            continue;
        }
        if (intch > 127) {
            // ignore UTF-8 characters
            continue;
        }
        if (isdigit(ch)) {
            continue;
        }
        posch = i < tsize - 1 ? nohidden.at(i + 1) : 0;
        if ((ch == '^') && (posch == '^')) {
            // use 6 slot in array for "^^" (heavy accent)
            ch = 6;
            intch = 6;
            articloc.at(intch) = i + 1;
            posch = i < tsize - 2 ? nohidden.at(i + 2) : 'g';
            i++;
            continue;
        }
        if ((ch == '"') && (posch == '"')) {
            // use 9 slot in array for ""^" (snap pizzicato)
            ch = 9;
            intch = 9;
            articloc.at(intch) = i + 1;
            posch = i < tsize - 2 ? nohidden.at(i + 2) : 'g';
            if (posch == m_signifiers.below) {
                articpos.at(intch) = -1;
            }
            else if (posch == m_signifiers.above) {
                articpos.at(intch) = +1;
            }
            i++;
            continue;
        }
        else if ((ch == '\'') && (posch == '\'')) {
            // staccatissimo alternate (eventually remove)
            ch = '`';
            intch = (unsigned char)ch;
            posch = i < tsize - 2 ? nohidden.at(i + 2) : 'g';
            i++;
        }
        else if ((ch == '~') && (posch == '~')) {
            // textual tenuto
            textTenuto = true;
            i++;
            posch = i < tsize - 1 ? nohidden.at(i + 1) : 0;
            if (m_signifiers.below && (posch == m_signifiers.below)) {
                textTenutoBelow = true;
            }
            continue;
        }
        if (m_signifiers.verticalStroke == ch) {
            // use 7 slot in array for vertical strokes
            ch = 7;
            intch = 7;
        }
        if (m_signifiers.lhpizz == ch) {
            // use 8 slot in array for left-hand pizzicato symbol
            ch = 8;
            intch = 8;
        }
        articloc.at(intch) = i + 1;
        if (nextch == 'X') {
            // Count explicit articulation duplications:
            articcount.at(intch)++;
        }

        if (posch) {
            // check for gestural articulations
            pos2ch = i < tsize - 2 ? nohidden.at(i + 2) : 0;
            pos3ch = i < tsize - 3 ? nohidden.at(i + 3) : 0;
            if ((posch == 'y') && (pos2ch != 'y')) {
                articges[ch] = 1;
            }
            else if (m_signifiers.above && (posch == m_signifiers.above) && (pos2ch == 'y') && (pos3ch != 'y')) {
                articges[ch] = 1;
            }
            else if (m_signifiers.below && (posch == m_signifiers.below) && (pos2ch == 'y') && (pos3ch != 'y')) {
                articges[ch] = 1;
            }
        }

        if ((posch != 0) && (posch == m_signifiers.above)) {
            articpos.at(intch) = 1;
            showpos.at(intch) = true;
        }
        else if ((posch != 0) && (posch == m_signifiers.below)) {
            articpos.at(intch) = -1;
            showpos.at(intch) = true;
        }
    }

    if (textTenuto) {
        std::string text = "ten.";
        std::string placement = "above";
        if (textTenutoBelow) {
            placement = "below";
        }
        bool bold = false;
        bool italic = true;
        int justification = 0;
        std::string color = "";
        int vgroup = 0;
        int staffindex = m_rkern[token->getTrack()];
        addDirection(text, placement, bold, italic, token, staffindex, justification, color, vgroup);
    }

    // second position is the staff position (-1=below, 0=undefined, 1=above)
    std::vector<data_ARTICULATION> artics;
    std::vector<int> positions;
    std::vector<bool> showingpositions;
    std::vector<int> gestural;
    std::vector<int> counts;

    // place articulations in stacking order (nearest to furthest from note):
    if (articloc['\'']) {
        artics.push_back(ARTICULATION_stacc);
        positions.push_back(articpos['\'']);
        gestural.push_back(articges['\'']);
        counts.push_back(articcount['\'']);
        showingpositions.push_back(showpos['\'']);
    }
    if (articloc['`']) {
        artics.push_back(ARTICULATION_stacciss);
        positions.push_back(articpos['`']);
        gestural.push_back(articges['`']);
        counts.push_back(articcount['`']);
        showingpositions.push_back(showpos['`']);
    }
    if (articloc['~']) {
        artics.push_back(ARTICULATION_ten);
        positions.push_back(articpos['~']);
        gestural.push_back(articges['~']);
        counts.push_back(articcount['~']);
        showingpositions.push_back(showpos['~']);
    }
    if (articloc[6]) {
        artics.push_back(ARTICULATION_marc);
        positions.push_back(articpos[6]);
        gestural.push_back(articges[6]);
        counts.push_back(articcount[6]);
        showingpositions.push_back(showpos[6]);
    }
    if (articloc[7]) {
        artics.push_back(ARTICULATION_stroke);
        positions.push_back(articpos[7]);
        gestural.push_back(articges[7]);
        counts.push_back(articcount[7]);
        showingpositions.push_back(showpos[7]);
    }
    if (articloc[8]) {
        artics.push_back(ARTICULATION_lhpizz);
        positions.push_back(articpos[8]);
        gestural.push_back(articges[8]);
        counts.push_back(articcount[8]);
        showingpositions.push_back(showpos[8]);
    }
    if (articloc[9]) {
        artics.push_back(ARTICULATION_snap);
        positions.push_back(articpos[9]);
        gestural.push_back(articges[9]);
        counts.push_back(articcount[9]);
        showingpositions.push_back(showpos[9]);
    }
    if (articloc['^']) {
        artics.push_back(ARTICULATION_acc);
        positions.push_back(articpos['^']);
        gestural.push_back(articges['^']);
        counts.push_back(articcount['^']);
        showingpositions.push_back(showpos['^']);
    }
    if (articloc['o']) {
        artics.push_back(ARTICULATION_harm);
        positions.push_back(articpos['o']);
        gestural.push_back(articges['o']);
        counts.push_back(articcount['o']);
        showingpositions.push_back(showpos['o']);
    }
    if (articloc['v']) {
        artics.push_back(ARTICULATION_upbow);
        positions.push_back(articpos['v']);
        gestural.push_back(articges['v']);
        counts.push_back(articcount['v']);
        showingpositions.push_back(showpos['v']);
    }
    if (articloc['u']) {
        artics.push_back(ARTICULATION_dnbow);
        positions.push_back(articpos['u']);
        gestural.push_back(articges['u']);
        counts.push_back(articcount['u']);
        showingpositions.push_back(showpos['u']);
    }

    if (artics.empty()) {
        return;
    }

    std::vector<Artic *> articlist;
    std::string color = getLoColor(token, "ART");
    for (int i = 0; i < (int)artics.size(); ++i) {
        // multiple same articulations are currently
        // placed at the chord level rather than
        // on the note level (which is probably a better
        // location which doing duplicate articulations).
        for (int j = 0; (j == 0) || (j < counts[i]); j++) {

            Artic *artic = new Artic();
            articlist.push_back(artic);
            element->AddChild(artic);

            if (artics.size() > 1) {
                // reorder ID numbers by sequence in token later
                setLocationId(artic, token, i + 1);
            }
            else {
                setLocationId(artic, token);
            }
            if (counts.at(i)) {
                std::string id = artic->GetID();
                id += "N";
                id += to_string(j + 1);
                artic->SetID(id);
            }
            if (!color.empty()) {
                artic->SetColor(color);
            }
            std::vector<data_ARTICULATION> oneartic;
            oneartic.clear();
            oneartic.push_back(artics.at(i));
            if (gestural.at(i)) {
                // can only add one gestural articulation?
                // artic->SetArticGes(artics.at(i));
                // artic->SetArticGes(onartic);
                continue;
            }
            if (ss[staffindex].suppress_articulations) {
                // artic->SetArticGes(artics.at(i));
                // artic->SetArticGes(onartic);
                continue;
            }
            else {
                artic->SetArtic(oneartic);
            }
            if (positions.at(i) > 0) {
                setPlaceRelEvent(artic, "above", positions.at(i));
            }
            else if (positions.at(i) < 0) {
                setPlaceRelEvent(artic, "below", positions.at(i));
            }
            // add gestural articulation info
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getDirection --
//    0  = no direction specified
//    1  = place above
//    -1 = place below
//

int HumdrumInput::getDirection(const std::string &token, const std::string &target)
{
    std::string newtarget;

    if (m_signifiers.above) {
        newtarget = target;
        newtarget.push_back(m_signifiers.above);
        if (token.find(newtarget) != std::string::npos) {
            return +1;
        }
    }

    if (m_signifiers.below) {
        newtarget = target;
        newtarget.push_back(m_signifiers.below);
        if (token.find(newtarget) != std::string::npos) {
            return -1;
        }
    }

    return 0;
}

//////////////////////////////
//
// HumdrumInput::embedPitchInformationInClass --
//

void HumdrumInput::embedPitchInformationInClass(Note *note, const std::string &token)
{
    if (token.find("r") != std::string::npos) {
        return;
    }
    if (token == ".") {
        return;
    }
    int base40 = hum::Convert::kernToBase40(token);
    int base40chroma = base40 % 40;
    int oct = base40 / 40;
    int acc = hum::Convert::base40ToAccidental(base40);
    int base12chroma = hum::Convert::base40ToMidiNoteNumber(base40) % 12;
    int base7chroma = hum::Convert::base40ToDiatonic(base40) % 7;
    std::string pname;
    switch (base7chroma) {
        case 0: pname = "c"; break;
        case 1: pname = "d"; break;
        case 2: pname = "e"; break;
        case 3: pname = "f"; break;
        case 4: pname = "g"; break;
        case 5: pname = "a"; break;
        case 6: pname = "b"; break;
    }

    std::string accid;
    switch (acc) {
        case 0: accid = "n"; break;
        case 1: accid = "s"; break;
        case -1: accid = "f"; break;
        case 2: accid = "ss"; break;
        case -2: accid = "ff"; break;
        case 3: accid = "sss"; break;
        case -3: accid = "fff"; break;
    }

    std::stringstream ss;
    ss << "pname-" << pname;
    ss << " ";
    ss << "acc-" << accid;
    ss << " ";
    ss << "oct-" << oct;
    ss << " ";
    ss << "b40c-" << base40chroma;
    ss << " ";
    ss << "b12c-" << base12chroma;
    ss << " ";
    appendTypeTag(note, ss.str());
}

//////////////////////////////
//
// HumdrumInput::embedQstampInClass --
//

void HumdrumInput::embedQstampInClass(Note *note, hum::HTp token, const std::string &tstring)
{
    hum::HumNum starttime = token->getDurationFromStart();
    hum::HumNum endtime = starttime + token->getDuration();
    std::stringstream sson;
    std::stringstream ssoff;
    sson << "qon-" << starttime.getNumerator();
    if (starttime.getDenominator() != 1) {
        sson << "_" << starttime.getDenominator();
    }
    ssoff << "qoff-" << endtime.getNumerator();
    if (endtime.getDenominator() != 1) {
        ssoff << "_" << endtime.getDenominator();
    }
    appendTypeTag(note, sson.str());
    appendTypeTag(note, ssoff.str());

    /*
            if (tstring.find("[") != std::string::npos) {
            hum::HumNum realendtime = starttime + token->getTiedDuration();
            std::stringstream ssofftied;
            ssofftied << "qoff-tied-" << realendtime.getNumerator();
            if (realendtime.getDenominator() != 1) {
                    ssoff << "_" << realendtime.getDenominator();
            }
            }
    */
}

//
// Rest version (rqon/rqoff variant)
//

void HumdrumInput::embedQstampInClass(Rest *rest, hum::HTp token, const std::string &tstring)
{
    hum::HumNum starttime = token->getDurationFromStart();
    hum::HumNum endtime = starttime + token->getDuration();
    std::stringstream sson;
    std::stringstream ssoff;
    sson << "rqon-" << starttime.getNumerator();
    if (starttime.getDenominator() != 1) {
        sson << "_" << starttime.getDenominator();
    }
    ssoff << "rqoff-" << endtime.getNumerator();
    if (endtime.getDenominator() != 1) {
        ssoff << "_" << endtime.getDenominator();
    }
    appendTypeTag(rest, sson.str());
    appendTypeTag(rest, ssoff.str());
}

//
// MRest version (rqon/rqoff variant)
//

void HumdrumInput::embedQstampInClass(MRest *mrest, hum::HTp token, const std::string &tstring)
{
    hum::HumNum starttime = token->getDurationFromStart();
    hum::HumNum endtime = starttime + token->getDuration();
    std::stringstream sson;
    std::stringstream ssoff;
    sson << "rqon-" << starttime.getNumerator();
    if (starttime.getDenominator() != 1) {
        sson << "_" << starttime.getDenominator();
    }
    ssoff << "rqoff-" << endtime.getNumerator();
    if (endtime.getDenominator() != 1) {
        ssoff << "_" << endtime.getDenominator();
    }
    appendTypeTag(mrest, sson.str());
    appendTypeTag(mrest, ssoff.str());
}

//
// Space version (rqon/rqoff variant)
//

void HumdrumInput::embedQstampInClass(Space *irest, hum::HTp token, const std::string &tstring)
{
    hum::HumNum starttime = token->getDurationFromStart();
    hum::HumNum endtime = starttime + token->getDuration();
    std::stringstream sson;
    std::stringstream ssoff;
    sson << "rqon-" << starttime.getNumerator();
    if (starttime.getDenominator() != 1) {
        sson << "_" << starttime.getDenominator();
    }
    ssoff << "rqoff-" << endtime.getNumerator();
    if (endtime.getDenominator() != 1) {
        ssoff << "_" << endtime.getDenominator();
    }
    appendTypeTag(irest, sson.str());
    appendTypeTag(irest, ssoff.str());
}

//////////////////////////////
//
// HumdrumInput::embedTieInformation --
//

void HumdrumInput::embedTieInformation(Note *note, const std::string &token)
{
    if (token.find("[") != std::string::npos) {
        appendTypeTag(note, "tie-start");
    }
    else if (token.find("]") != std::string::npos) {
        appendTypeTag(note, "tie-stop");
    }
    else if (token.find("_") != std::string::npos) {
        appendTypeTag(note, "tie-cont");
    }
}

//////////////////////////////
//
// HumdrumInput::colorNote --
//

void HumdrumInput::colorNote(Note *note, hum::HTp token, const std::string &subtoken, int line, int field)
{
    std::string spinecolor = getSpineColor(line, field);
    if (spinecolor != "") {
        note->SetColor(spinecolor);
    }

    if (m_mens) {
        int justification = 0;
        for (int i = 0; i < (int)m_signifiers.mens_mark.size(); ++i) {
            if (subtoken.find(m_signifiers.mens_mark[i]) != std::string::npos) {
                note->SetColor(m_signifiers.mens_mcolor[i]);
                appendTypeTag(note, "color-marked");
                if (!m_signifiers.mens_markdir[i].empty()) {
                    bool bold = true;
                    bool italic = false;
                    int staffindex = m_rkern[token->getTrack()];
                    addDirection(m_signifiers.mens_markdir[i], "above", bold, italic, token, staffindex, justification,
                        m_signifiers.mens_mcolor[i]);
                }
                break;
            }
        }
    }
    else {
        int justification = 0;
        std::vector<std::string> markcolors;
        markcolors.clear();
        for (int i = 0; i < (int)m_signifiers.mark.size(); ++i) {
            if (subtoken.find(m_signifiers.mark[i]) != std::string::npos) {
                markcolors.push_back(m_signifiers.mcolor[i]);
                appendTypeTag(note, "color-marked");
                if (!m_signifiers.markdir[i].empty()) {
                    bool bold = true;
                    bool italic = false;
                    int staffindex = m_rkern[token->getTrack()];
                    addDirection(m_signifiers.markdir[i], "above", bold, italic, token, staffindex, justification,
                        m_signifiers.mcolor[i]);
                }
            }
            if (markcolors.size() == 1) {
                note->SetColor(markcolors[0]);
            }
            else if (markcolors.size() > 1) {
                hum::PixelColor a;
                hum::PixelColor b;
                a.setColor(markcolors[0]);
                b.setColor(markcolors[1]);
                hum::PixelColor mixed = hum::PixelColor::mix(a, b);
                for (int i = 2; i < (int)markcolors.size(); ++i) {
                    a.setColor(markcolors[i]);
                    mixed = hum::PixelColor::mix(mixed, a);
                }
                std::string mixedcolor = mixed.getHexColor();
                note->SetColor(mixedcolor);
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::colorVerse --
//

void HumdrumInput::colorVerse(Verse *verse, std::string &token)
{
    hum::HumRegex hre;
    for (int i = 0; i < (int)m_signifiers.textmark.size(); ++i) {
        if (token.find(m_signifiers.textmark[i]) != std::string::npos) {
            verse->SetColor(m_signifiers.textcolor[i]);
            // appendTypeTag(verse, "marked");
            // remove mark character from text (so that it does not display):
            std::string mystring;
            mystring += m_signifiers.textmark[i];
            hre.replaceDestructive(token, "", mystring, "g");
            return;
        }
    }
    if (m_signifiers.mark.empty()) {
        return;
    }

    // force the verse text black so that it is not
    // highlighted when notes are highlighted.
    verse->SetColor("black");
}

//////////////////////////////
//
// setPlaceRelStaff --
//

template <class ELEMENT> void HumdrumInput::setPlaceRelStaff(ELEMENT *element, const std::string &place, bool showplace)
{
    element->SetPlace(element->AttPlacementRelStaff::StrToStaffrel(place));
    if (m_humtype && showplace) {
        appendTypeTag(element, "placed");
    }
}

template <class ELEMENT> void HumdrumInput::setPlaceRelEvent(ELEMENT *element, const std::string &place, bool showplace)
{
    element->SetPlace(element->AttPlacementRelEvent::StrToStaffrel(place));
    if (m_humtype && showplace) {
        appendTypeTag(element, "placed");
    }
}

/////////////////////////////
//
// HumdrumInput::verticalRest -- If the rest has pitch information, set the vertical
//    position of the rest from the pitch.
//

template <class ELEMENT> void HumdrumInput::verticalRest(ELEMENT element, const std::string &token)
{
    return; // calculating this in humlib now.
    /*
        hum::HumRegex hre;
        if (!hre.search(token, "([A-Ga-g]+)")) {
            return;
        }
        std::string result = hre.getMatch(1);

        int base40 = hum::Convert::kernToBase40(result);
        int oct = base40 / 40;
        int base7chroma = hum::Convert::base40ToDiatonic(base40) % 7;
        switch (base7chroma) {
            case 0: element->SetPloc(PITCHNAME_c); break;
            case 1: element->SetPloc(PITCHNAME_d); break;
            case 2: element->SetPloc(PITCHNAME_e); break;
            case 3: element->SetPloc(PITCHNAME_f); break;
            case 4: element->SetPloc(PITCHNAME_g); break;
            case 5: element->SetPloc(PITCHNAME_a); break;
            case 6: element->SetPloc(PITCHNAME_b); break;
        }

        element->SetOloc(oct);
    */
}

//////////////////////////////
//
// HumdrumInput::colorRest --
//

void HumdrumInput::colorRest(Rest *rest, const std::string &token, int line, int field)
{
    std::string spinecolor;
    if ((line >= 0) && (field >= 0)) {
        spinecolor = getSpineColor(line, field);
    }
    if (spinecolor != "") {
        rest->SetColor(spinecolor);
    }

    if (m_mens) {
        for (int i = 0; i < (int)m_signifiers.mens_mark.size(); ++i) {
            if (token.find(m_signifiers.mens_mark[i]) != std::string::npos) {
                rest->SetColor(m_signifiers.mens_mcolor[i]);
                break;
            }
        }
    }
    else {
        for (int i = 0; i < (int)m_signifiers.mark.size(); ++i) {
            if (token.find(m_signifiers.mark[i]) != std::string::npos) {
                rest->SetColor(m_signifiers.mcolor[i]);
                break;
            }
        }
    }
    if (token.find("yy") != std::string::npos) {
        // invisible rest that was probably sent here specifically be colored
        if (!m_signifiers.irest_color.empty()) {
            rest->SetColor(m_signifiers.irest_color);
        }
        else if (!m_signifiers.space_color.empty()) {
            rest->SetColor(m_signifiers.space_color);
        }
    }
    else if (token == "") {
        // implicit space that should be colored as a rest
        if (!m_signifiers.ispace_color.empty()) {
            rest->SetColor(m_signifiers.ispace_color);
        }
        else if (!m_signifiers.space_color.empty()) {
            rest->SetColor(m_signifiers.space_color);
        }
    }
    else if (token.find("r") == std::string::npos) {
        // explicit space that should be colored as a rest
        if (!m_signifiers.rspace_color.empty()) {
            rest->SetColor(m_signifiers.rspace_color);
        }
        else if (!m_signifiers.space_color.empty()) {
            rest->SetColor(m_signifiers.space_color);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getSpineColor --  But suppress black colors which are
//     the default color of notes.
//

string HumdrumInput::getSpineColor(int line, int field)
{
    hum::HumdrumFile &infile = m_infiles[0];
    std::string output;
    int track = infile.token(line, field)->getTrack();
    int strack = infile.token(line, field)->getSubtrack();
    if (!m_spine_color[track].at(strack).empty()) {
        if ((m_spine_color[track].at(strack) != "black") && (m_spine_color[track].at(strack) != "#000000")
            && (m_spine_color[track].at(strack) != "#000")) {
            output = m_spine_color[track].at(strack);
        }
    }
    if (!m_has_color_spine) {
        return output;
    }
    for (int i = field + 1; i < infile[line].getFieldCount(); ++i) {
        if (!infile.token(line, i)->isDataType("**color")) {
            continue;
        }
        output = *infile.token(line, i)->resolveNull();
        if (output == ".") {
            output = "";
        }
        else if (output == "black") {
            output = "";
        }
        else if (output == "#000000") {
            output = "";
        }
        else if (output == "#000") {
            output = "";
        }
        break;
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::addOrnamentMarkers -- Temporarily convert
//   mordents and trills to text markers (<dir>) for DH.
//   These markers will overwrite any other existing text directions.
//

void HumdrumInput::addOrnamentMarkers(hum::HTp token)
{
    if (!token) {
        return;
    }

    if (strchr(token->c_str(), 'O') != NULL) { // generic ornament
        token->setValue("LO", "TX", "t", "*");
        token->setValue("LO", "TX", "a", "true");
    }
}

//////////////////////////////
//
// HumdrumInput::addSpace -- Add one or more space elements
//    to match the required duration.
//    default value: typestring = ""
//

void HumdrumInput::addSpace(std::vector<std::string> &elements, std::vector<void *> &pointers, hum::HumNum duration,
    const std::string &typestring)
{
    bool visible = false;
    if ((!m_signifiers.ispace_color.empty()) || (!m_signifiers.space_color.empty())) {
        visible = true;
    }

    while (duration > 0) {
        if (visible) {
            Rest *rest = new Rest();
            // setLocationId(rest, layerdata[i]);
            // convertRest(rest, layerdata[i], -1, staffindex);
            // processSlurs(layerdata[i]);
            // processPhrases(layerdata[i]);
            // processDynamics(layerdata[i], staffindex);
            // processDirections(layerdata[i], staffindex);
            // int line = layerdata[i]->getLineIndex();
            // int field = layerdata[i]->getFieldIndex();
            // colorRest(rest, "", line, field);
            colorRest(rest, "", -1, -1);
            appendElement(elements, pointers, rest);
            duration -= setDuration(rest, duration);
            if (!typestring.empty()) {
                rest->SetType(typestring);
            }
        }
        else {
            Space *space = new Space();
            // if (m_doc->GetOptions()->m_humType.GetValue()) {
            //    embedQstampInClass(space, layerdata[i], *layerdata[i]);
            // }
            appendElement(elements, pointers, space);
            duration -= setDuration(space, duration);
            if (!typestring.empty()) {
                space->SetType(typestring);
            }
        }
    }
}

//////////////////////////////
//
// processTerminalLong -- Not set up for chords yet.
//

void HumdrumInput::processTerminalLong(hum::HTp token)
{
    if (!m_signifiers.terminallong) {
        return;
    }
    if (token->find(m_signifiers.terminallong) == std::string::npos) {
        return;
    }
    std::string doublelong;
    doublelong += m_signifiers.terminallong;
    doublelong += m_signifiers.terminallong;
    if (token->find(doublelong) != std::string::npos) {
        token->setValue("LO", "N", "vis", "000");
    }
    else {
        token->setValue("LO", "N", "vis", "00");
    }
    if ((token->find('[') != std::string::npos) || (token->find('_') != std::string::npos)) {
        removeCharacter(token, '[');
        removeCharacter(token, '_');

        int pitch = hum::Convert::kernToBase40(token);
        hum::HTp testtok = token->getNextToken();
        while (testtok) {
            if (testtok->isBarline()) {
                // make measure invisible:
                testtok->setText(*testtok + "-");
            }
            else if (testtok->isData()) {
                if (testtok->isNull()) {
                    testtok = testtok->getNextToken();
                    continue;
                }
                int tpitch = hum::Convert::kernToBase40(testtok);
                if (tpitch != pitch) {
                    break;
                }
                if ((testtok->find(']') == std::string::npos) && (testtok->find('_') == std::string::npos)) {
                    break;
                }
                // make note invisible:
                testtok->setText(*testtok + "yy");
                if (testtok->find("_") != std::string::npos) {
                    removeCharacter(testtok, '_');
                    testtok = testtok->getNextToken();
                    continue;
                }
                else if (testtok->find("]") != std::string::npos) {
                    removeCharacter(testtok, ']');
                    break;
                }
            }
            testtok = testtok->getNextToken();
        }
    }
    // If token is tied, then follow ties to attached notes and make invisible.
}

//////////////////////////////
//
// processTerminalBreve -- Not set up for chords yet.
//

void HumdrumInput::processTerminalBreve(hum::HTp token)
{
    if (!m_signifiers.terminalbreve) {
        return;
    }
    if (token->find(m_signifiers.terminalbreve) == std::string::npos) {
        return;
    }
    token->setValue("LO", "N", "vis", "0");
    if ((token->find('[') != std::string::npos) || (token->find('_') != std::string::npos)) {
        removeCharacter(token, '[');
        removeCharacter(token, '_');

        int pitch = hum::Convert::kernToBase40(token);
        hum::HTp testtok = token->getNextToken();
        while (testtok) {
            if (testtok->isBarline()) {
                // make measure invisible:
                testtok->setText(*testtok + "-");
            }
            else if (testtok->isData()) {
                if (testtok->isNull()) {
                    testtok = testtok->getNextToken();
                    continue;
                }
                int tpitch = hum::Convert::kernToBase40(testtok);
                if (tpitch != pitch) {
                    break;
                }
                if ((testtok->find(']') == std::string::npos) && (testtok->find('_') == std::string::npos)) {
                    break;
                }
                // make note invisible:
                testtok->setText(*testtok + "yy");
                if (testtok->find("_") != std::string::npos) {
                    removeCharacter(testtok, '_');
                    testtok = testtok->getNextToken();
                    continue;
                }
                else if (testtok->find("]") != std::string::npos) {
                    removeCharacter(testtok, ']');
                    break;
                }
            }
            testtok = testtok->getNextToken();
        }
    }
    // If token is tied, then follow ties to attached notes and make invisible.
}

//////////////////////////////
//
// HumdrumInput::processOverfillingNotes -- Shorten the gestural duration of notes
//     that overfill the measure, but keep the visual display of the
//     duration the same.  The chopped note durations in the succeeding
//     measure(s) will be converted into spaces.  This will cause problems
//     with the MIDI file rendering since the notes will be cut short.
//     Ideally ties should be allowed to become invisible, and since notes
//     are already allowed to become invisible, eventually convert the
//     chopped off pieces of the overfilling note into a series of invisible
//     tied notes.
//
// See issue https://github.com/music-encoding/music-encoding/issues/469
//

bool HumdrumInput::processOverfillingNotes(hum::HTp token)
{
    hum::HumNum duration = token->getDuration();
    hum::HumNum barend = token->getDurationToBarline();
    if (barend == 0) {
        // This can happen due to unterminated measure.
        // In such a case, the note/rest cannot overfill the
        // the measure since there is not measure end.
        return false;
    }
    if (duration <= barend) {
        return false;
    }

    bool nextbarignore = isNextBarIgnored(token);
    if (nextbarignore) {
        // Ignore invisible barlines that need to be skipped over since they
        // will not be converted into MEI data.  Only the first barline
        // will be considered, but all barlines through the end of the duration
        // of the note should be checked in the general solution.
        return false;
    }

    std::string logical_rhythm = hum::Convert::durationToRecip(barend);
    std::string visual_rhythm = hum::Convert::kernToRecip(token);
    token->setValue("auto", "N", "vis", visual_rhythm);
    token->setValue("auto", "MEI", "dur.logical", logical_rhythm);
    token->setValue("auto", "MEI", "type", "straddle");
    return true;
}

//////////////////////////////
//
// HumdrumInput::isNextBarIgnored --
//

bool HumdrumInput::isNextBarIgnored(hum::HTp token)
{
    hum::HTp current = token->getNextToken();
    while (current && !current->isBarline()) {
        if (current->isNull()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isData()) {
            break;
        }
        current = current->getNextToken();
    }
    if (!current) {
        return false;
    }
    if (!current->isBarline()) {
        return false;
    }
    if (current->allSameBarlineStyle()) {
        return false;
    }
    if (current->find('-') == std::string::npos) {
        return false;
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput:: removeCharacter --
//

void HumdrumInput::removeCharacter(hum::HTp token, char removechar)
{
    std::string output;
    for (char ch : *token) {
        if (ch == removechar) {
            continue;
        }
        output += ch;
    }

    token->setText(output);
}

//////////////////////////////
//
// HumdrumInput::processChordSignifiers --
//

void HumdrumInput::processChordSignifiers(Chord *chord, hum::HTp token, int staffindex)
{
    if (m_signifiers.nostem && token->find(m_signifiers.nostem) != std::string::npos) {
        chord->SetStemVisible(BOOLEAN_false);
    }

    if (m_signifiers.cuesize) {
        int tcount = 1;
        int cuecount = 0;
        for (int i = 0; i < (int)token->size(); ++i) {
            if ((*token)[i] == m_signifiers.cuesize) {
                cuecount++;
            }
            if ((*token)[i] == ' ') {
                tcount++;
            }
        }
        if ((cuecount > 0) && (tcount == cuecount)) {
            chord->SetCue(BOOLEAN_true);
        }
    }
    else if (m_staffstates.at(staffindex).cue_size.at(m_currentlayer)) {
        chord->SetCue(BOOLEAN_true);
    }

    processTerminalLong(token); // Not tested and probably won't work yet on chords.
    processTerminalBreve(token); // Not tested and probably won't work yet on chords.
}

//////////////////////////////
//
// HumdrumInput::processGlobalDirections --
//

void HumdrumInput::processGlobalDirections(hum::HTp token, int staffindex)
{
    const std::vector<hum::HTp> &staffstarts = m_staffstarts;
    int staffcount = (int)staffstarts.size();

    // only put above or below system (no cases of intermediate
    // directions allowed for now)
    if ((staffindex > 0) && (staffindex < staffcount - 1)) {
        return;
    }

    hum::HumdrumLine *hline = token->getOwner();
    if (hline == NULL) {
        return;
    }

    // Generalize the code below to not duplicate processDirections():

    std::string text = hline->getValue("LO", "TX", "t");
    if (text.size() == 0) {
        return;
    }

    bool zparam = hline->isDefined("LO", "TX", "Z");
    bool yparam = hline->isDefined("LO", "TX", "Y");

    bool aparam = hline->getValueBool("LO", "TX", "a"); // place above staff
    bool bparam = false;
    bool cparam = false;
    if (!aparam) {
        bparam = hline->getValueBool("LO", "TX", "b"); // place below staff
    }
    if (!aparam && !bparam) {
        cparam = hline->getValueBool("LO", "TX", "c"); // place below staff
    }

    // default font for text string (later check for embedded fonts)
    bool italic = false;
    bool bold = false;

    int vgroup = -1;
    if (hline->isDefined("LO", "TX", "vgrp")) { // italic
        vgroup = hline->getValueInt("LO", "TX", "vgrp");
    }
    else if (hline->isDefined("LO", "TX", "vg")) {
        vgroup = hline->getValueInt("LO", "TX", "vg");
    }

    if (hline->isDefined("LO", "TX", "i")) { // italic
        italic = true;
    }
    if (hline->isDefined("LO", "TX", "B")) { // bold
        bold = true;
    }
    if (hline->isDefined("LO", "TX", "bi")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (hline->isDefined("LO", "TX", "ib")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (hline->isDefined("LO", "TX", "Bi")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (hline->isDefined("LO", "TX", "iB")) { // italic
        bold = true;
        italic = true;
    }

    bool tempo = hline->isDefined("LO", "TX", "tempo");

    double Y = 0.0;
    double Z = 0.0;
    bool showplace = false;
    std::string placement;
    if (aparam) {
        placement = "above";
        showplace = true;
    }
    else if (bparam) {
        placement = "below";
        showplace = true;
    }
    else if (cparam) {
        placement = "between";
        showplace = true;
    }
    else if (zparam) {
        Z = hline->getValueInt("LO", "TX", "Z");
        if (Z >= 0) {
            placement = "above";
            showplace = true;
        }
        else {
            placement = "below";
            showplace = true;
        }
    }
    else if (yparam) {
        Y = hline->getValueInt("LO", "TX", "Y");
        if (Y >= 0) {
            placement = "below";
            showplace = true;
        }
        else {
            placement = "above";
            showplace = true;
        }
    }
    else {
        placement = "above";
    }

    if (tempo) {
        Tempo *tempo = new Tempo();
        double midibpm = getMmTempo(token);
        if (midibpm > 0) {
            tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
        }
        if (cparam) {
            setStaffBetween(tempo, m_currentstaff);
        }
        else {
            setStaff(tempo, m_currentstaff);
        }
        setLocationId(tempo, token);
        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
        tempo->SetTstamp(tstamp.getFloat());
        if (placement == "above") {
            setPlaceRelStaff(tempo, "above", showplace);
            addChildBackMeasureOrSection(tempo);
        }
        else if (placement == "below") {
            setPlaceRelStaff(tempo, "below", showplace);
            addChildMeasureOrSection(tempo);
        }
        else if (placement == "between") {
            setPlaceRelStaff(tempo, "between", showplace);
            addChildMeasureOrSection(tempo);
        }
        else {
            addChildMeasureOrSection(tempo);
        }
        if ((!italic) || bold) {
            Rend *rend = new Rend();
            tempo->AddChild(rend);
            addTextElement(rend, text);
            if (!italic) {
                rend->SetFontstyle(FONTSTYLE_normal);
            }
            else {
                // Explicitly set italic fontstyle.
                rend->SetFontstyle(FONTSTYLE_italic);
            }
            if (bold) {
                rend->SetFontweight(FONTWEIGHT_bold);
            }
        }
        else {
            addTextElement(tempo, text);
        }
    }
    else {

        Dir *dir = new Dir();
        if (cparam) {
            setStaffBetween(dir, m_currentstaff);
        }
        else {
            setStaff(dir, m_currentstaff);
        }
        setLocationId(dir, token);
        // setAttachmentType(dir, token);
        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
        dir->SetTstamp(tstamp.getFloat());

        if (vgroup > 0) {
            dir->SetVgrp(vgroup);
        }

        if (placement == "above") {
            setPlaceRelStaff(dir, "above", showplace);
            addChildBackMeasureOrSection(dir);
        }
        else if (placement == "below") {
            setPlaceRelStaff(dir, "below", showplace);
            addChildMeasureOrSection(dir);
        }
        else if (placement == "between") {
            setPlaceRelStaff(dir, "between", showplace);
            addChildMeasureOrSection(dir);
        }
        else {
            addChildMeasureOrSection(dir);
        }
        if ((!italic) || bold) {
            Rend *rend = new Rend();
            dir->AddChild(rend);
            addTextElement(rend, text);
            if (!italic) {
                rend->SetFontstyle(FONTSTYLE_normal);
            }
            else {
                // Explicitly set italic fontstyle.
                rend->SetFontstyle(FONTSTYLE_italic);
            }
            if (bold) {
                rend->SetFontweight(FONTWEIGHT_bold);
            }
        }
        else {
            addTextElement(dir, text);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::processDirections --
//

void HumdrumInput::processDirections(hum::HTp token, int staffindex)
{

    int lcount = token->getLinkedParameterSetCount();
    for (int i = 0; i < lcount; ++i) {
        processLinkedDirection(i, token, staffindex);
    }

    // process an internal text direction (no longer common):
    std::string text = token->getValue("LO", "TX", "t");
    if (text.size() == 0) {
        return;
    }

    // justification == 0 means no explicit justification (mostly left justified)
    // justification == 1 means right justified
    // justification == 2 means center justified
    int justification = 0;
    if (token->isDefined("LO", "TX", "rj")) {
        justification = 1;
    }
    else if (token->isDefined("LO", "TX", "cj")) {
        justification = 2;
    }

    bool zparam = token->isDefined("LO", "TX", "Z");
    bool yparam = token->isDefined("LO", "TX", "Y");

    bool aparam = token->getValueBool("LO", "TX", "a"); // place above staff
    bool bparam = false;
    bool cparam = false;
    if (!aparam) {
        bparam = token->getValueBool("LO", "TX", "b"); // place below staff
    }
    if (!aparam && !bparam) {
        cparam = token->getValueBool("LO", "TX", "c"); // place below staff, centered with next one
    }

    // default font for text string (later check for embedded fonts)
    bool italic = false;
    bool bold = false;

    int vgroup = -1;
    if (token->isDefined("LO", "TX", "vgrp")) { // italic
        vgroup = token->getValueInt("LO", "TX", "vgrp");
    }
    else if (token->isDefined("LO", "TX", "vg")) { // italic
        vgroup = token->getValueInt("LO", "TX", "vg");
    }

    if (token->isDefined("LO", "TX", "i")) { // italic
        italic = true;
    }
    if (token->isDefined("LO", "TX", "B")) { // bold
        bold = true;
    }
    if (token->isDefined("LO", "TX", "bi")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (token->isDefined("LO", "TX", "ib")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (token->isDefined("LO", "TX", "Bi")) { // bold-italic
        bold = true;
        italic = true;
    }
    if (token->isDefined("LO", "TX", "iB")) { // italic
        bold = true;
        italic = true;
    }

    std::string color = token->getValue("LO", "TX", "color");

    double Y = 0.0;
    double Z = 0.0;
    std::string placement;
    if (aparam) {
        placement = "above";
    }
    else if (bparam) {
        placement = "below";
    }
    else if (cparam) {
        placement = "between";
    }

    else if (zparam) {
        Z = token->getValueInt("LO", "TX", "Z");
        if (Z >= 0) {
            placement = "above";
        }
        else {
            placement = "below";
        }
    }
    else if (yparam) {
        Y = token->getValueInt("LO", "TX", "Y");
        if (Y >= 0) {
            placement = "below";
        }
        else {
            placement = "above";
        }
    }
    else {
        placement = "above";
    }

    addDirection(text, placement, bold, italic, token, staffindex, justification, color, vgroup);
}

//////////////////////////////
//
// HumdrumInput::isFirstTokenOnStaff -- Used to control global
//     directions: only one token will be used to generate a direction.
//

bool HumdrumInput::isFirstTokenOnStaff(hum::HTp token)
{
    int target = token->getTrack();
    int track;
    hum::HTp tok = token->getPreviousFieldToken();
    while (tok != NULL) {
        track = tok->getTrack();
        if (track != target) {
            return true;
        }
        if (tok->isNull()) {
            // need to check further
        }
        else {
            return false;
        }
        tok = tok->getPreviousFieldToken();
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::processLinkedDirection --
//

void HumdrumInput::processLinkedDirection(int index, hum::HTp token, int staffindex)
{
    bool globalQ = token->linkedParameterIsGlobal(index);
    bool firstQ = true;
    if (globalQ) {
        firstQ = isFirstTokenOnStaff(token);
    }

    if (!firstQ) {
        // Don't insert multiple global directions.
        return;
    }

    hum::HumParamSet *hps = token->getLinkedParameterSet(index);
    if (hps == NULL) {
        return;
    }

    if (hps->getNamespace1() != "LO") {
        return;
    }

    std::string namespace2 = hps->getNamespace2();
    bool textQ = namespace2 == "TX";
    bool sicQ = namespace2 == "SIC";
    int vgroup = -1;

    if (!(textQ || sicQ)) {
        // not a text direction so ignore
        return;
    }

    // default font for text string (later check for embedded fonts)
    bool italic = false;
    bool bold = false;
    bool zparam = false;
    bool yparam = false;
    bool aparam = false;
    bool bparam = false;
    bool cparam = false;

    // maybe add center justification as an option later
    // justification == 0 means no explicit justification (mostly left justified)
    // justification == 1 means right justified
    int justification = 0;

    if (token->isBarline()) {
        hum::HumNum startdur = token->getDurationFromStart();
        hum::HumdrumFile *hfile = token->getOwner()->getOwner();
        hum::HumNum totaldur = (*hfile)[hfile->getLineCount() - 1].getDurationFromStart();
        if (startdur == totaldur) {
            justification = 1;
        }
    }

    std::string color;
    if (sicQ) {
        // default color for sic text directions (set to black if not wanted)
        color = "limegreen";
    }

    bool problemQ = false;
    bool verboseQ = false;
    bool tempoQ = false;
    std::string text;
    std::string key;
    std::string value;
    std::string typevalue;
    std::string verboseType;
    std::string ovalue;
    std::string svalue;
    Dir *dir = NULL;
    Tempo *tempo = NULL;

    for (int i = 0; i < hps->getCount(); ++i) {
        key = hps->getParameterName(i);
        value = hps->getParameterValue(i);
        if (key == "a") {
            aparam = true;
        }
        else if (key == "b") {
            bparam = true;
        }
        else if (key == "c") {
            cparam = true;
        }
        else if (key == "t") {
            text = value;
            if (text.size() == 0) {
                // nothing to display
                return;
            }
        }
        else if (key == "Y") {
            yparam = true;
        }
        else if (key == "Z") {
            zparam = true;
        }
        else if (key == "i") {
            italic = true;
        }
        else if (key == "B") {
            bold = true;
        }
        else if (key == "Bi") {
            italic = true;
            bold = true;
        }
        else if (key == "bi") {
            italic = true;
            bold = true;
        }
        else if (key == "iB") {
            italic = true;
            bold = true;
        }
        else if (key == "ib") {
            italic = true;
            bold = true;
        }

        if (key == "rj") {
            justification = 1;
        }
        if (key == "cj") {
            justification = 2;
        }
        if (key == "color") {
            color = value;
        }
        if (key == "v") {
            verboseQ = true;
            verboseType = value;
        }
        if (key == "o") {
            ovalue = value;
        }
        if (key == "s") {
            svalue = value;
        }
        if (key == "problem") {
            problemQ = true;
        }
        if (key == "type") {
            typevalue = value;
        }
        if (key == "tempo") {
            tempoQ = true;
        }
        if (key == "vgrp") {
            if ((!value.empty()) && std::isdigit(value[0])) {
                vgroup = std::stoi(value);
            }
        }
        else if (key == "vg") {
            if ((!value.empty()) && std::isdigit(value[0])) {
                vgroup = std::stoi(value);
            }
        }
    }

    if ((namespace2 == "SIC") && !verboseQ) {
        return;
    }

    double Y = 0.0;
    double Z = 0.0;
    std::string placement;
    bool showplace = false;
    if (aparam) {
        placement = "above";
        showplace = true;
    }
    else if (bparam) {
        placement = "below";
        showplace = true;
    }
    else if (cparam) {
        placement = "between";
        showplace = true;
    }
    else if (zparam) {
        Z = token->getValueInt("LO", "TX", "Z");
        if (Z >= 0) {
            placement = "above";
            showplace = true;
        }
        else {
            placement = "below";
            showplace = true;
        }
    }
    else if (yparam) {
        Y = token->getValueInt("LO", "TX", "Y");
        if (Y >= 0) {
            placement = "below";
            showplace = true;
        }
        else {
            placement = "above";
            showplace = true;
        }
    }
    else {
        placement = "above";
    }

    if (sicQ) {
        if (verboseType == "text") {
            if (!ovalue.empty()) {
                text = ovalue;
            }
            else if (!svalue.empty()) {
                text = svalue;
            }
            else {
                text = "S";
            }
        }
        else {
            text = "S";
        }
    }

    int maxstaff = (int)m_staffstarts.size() - 1;

    if (token->linkedParameterIsGlobal(index)) {
        if ((placement == "below") && (staffindex != maxstaff)) {
            // For system-text, do not place on any staff except the bottom staff.
            // This will probably change in the future to place at the bottom
            // of each staff group only.
            return;
        }
        else if ((placement == "above") && (staffindex != 0)) {
            // For system-text, do not place on any staff except the top staff.
            // This will probably change in the future to place at the top
            // of each staff group only.
            return;
        }
    }

    hum::HumRegex hre;
    if (hre.search(text, "\\[[^=]*\\]\\s*=\\s*\\d+")) {
        int status = addTempoDirection(text, placement, bold, italic, token, staffindex, justification, color);
        if (status) {
            return;
        }
    }
    if (token->isTimeSignature()) {
        addTempoDirection(text, placement, bold, italic, token, staffindex, justification, color);
        return;
    }

    double midibpm = 0.0;
    if (tempoQ) {
        midibpm = getMmTempo(token, true);
        if (midibpm == 0) {
            // this is a redundant tempo message, so ignore (event as text dir).
            return;
        }
    }

    if (tempoQ) {

        tempo = new Tempo();
        if (midibpm > 0) {
            tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
        }
        if (placement == "between") {
            setStaffBetween(tempo, m_currentstaff);
        }
        else {
            setStaff(tempo, m_currentstaff);
        }
        hum::HTp dirtok = hps->getToken();
        if (dirtok != NULL) {
            setLocationId(tempo, dirtok);
        }
        else {
            cerr << "DIRTOK FOR " << token << " IS EMPTY " << endl;
        }
        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
        if (token->isMensLike()) {
            // Attach to note, not with measure timestamp.
            // Need to handle text on chords (will currently have a problem attaching to chords)
            std::string startid = getLocationId("note", token);
            tempo->SetStartid("#" + startid);
        }
        else {
            tempo->SetTstamp(tstamp.getFloat());
        }
        if (problemQ) {
            appendTypeTag(tempo, "problem");
        }
        if (sicQ) {
            appendTypeTag(tempo, "sic");
        }
        if (!typevalue.empty()) {
            appendTypeTag(tempo, typevalue);
        }
        addChildMeasureOrSection(tempo);
        if (placement == "above") {
            setPlaceRelStaff(tempo, "above", showplace);
        }
        else if (placement == "below") {
            setPlaceRelStaff(tempo, "below", showplace);
        }
        else if (placement == "between") {
            setPlaceRelStaff(tempo, "between", showplace);
        }
    }
    else {

        dir = new Dir();
        if (placement == "between") {
            setStaffBetween(dir, m_currentstaff);
        }
        else {
            setStaff(dir, m_currentstaff);
        }
        hum::HTp dirtok = hps->getToken();
        if (dirtok != NULL) {
            setLocationId(dir, dirtok);
        }
        else {
            cerr << "DIRTOK FOR " << token << " IS EMPTY " << endl;
        }

        if (token->isMensLike()) {
            attachToToken(dir, token);
        }
        else {
            setAttachmentType(dir, token);
        }

        // bool problemQ = false;
        // bool sicQ = false;
        if (vgroup > 0) {
            dir->SetVgrp(vgroup);
        }
        if (problemQ) {
            appendTypeTag(dir, "problem");
        }
        if (sicQ) {
            appendTypeTag(dir, "sic");
        }
        if (!typevalue.empty()) {
            appendTypeTag(dir, typevalue);
        }
        addChildMeasureOrSection(dir);
        if (placement == "above") {
            setPlaceRelStaff(dir, "above", showplace);
        }
        else if (placement == "below") {
            setPlaceRelStaff(dir, "below", showplace);
        }
        else if (placement == "between") {
            setPlaceRelStaff(dir, "between", showplace);
        }
    }

    if (problemQ) {
        italic = false;
        bold = false;
    }

    bool plain = !(italic || bold);
    bool needrend = italic || plain || bold || justification || color.size();
    bool onlysmufl = false;
    if (hre.search(text, "^(\\[.*?\\])+$")) {
        onlysmufl = true;
        needrend = justification || color.size();
    }

    if (needrend) {
        Rend *rend = new Rend();
        if (!color.empty()) {
            rend->SetColor(color);
        }
        else if (problemQ) {
            rend->SetColor("red");
        }
        else if (sicQ) {
            rend->SetColor("limegreen");
        }
        if (tempoQ && tempo) {
            tempo->AddChild(rend);
        }
        else if (dir) {
            dir->AddChild(rend);
        }
        addTextElement(rend, text);
        if (!italic) {
            rend->SetFontstyle(FONTSTYLE_normal);
        }
        else {
            // Explicitly set italic fontstyle.
            rend->SetFontstyle(FONTSTYLE_italic);
        }
        if (bold) {
            rend->SetFontweight(FONTWEIGHT_bold);
        }
        if (justification == 1) {
            rend->SetHalign(HORIZONTALALIGNMENT_right);
        }
        else if (justification == 2) {
            rend->SetHalign(HORIZONTALALIGNMENT_center);
        }
    }
    else {

        if (tempoQ && tempo) {
            addTextElement(tempo, text);
            if (onlysmufl && needrend) {
                int count = tempo->GetChildCount();
                for (int j = 0; j < count; j++) {
                    Object *obj = tempo->GetChild(j);
                    if (obj->GetClassName() != "Rend") {
                        continue;
                    }
                    Rend *item = (Rend *)obj;
                    if (!color.empty()) {
                        item->SetColor(color);
                    }
                    else if (problemQ) {
                        item->SetColor("red");
                    }
                    else if (sicQ) {
                        item->SetColor("limegreen");
                    }
                    if (!italic) {
                        item->SetFontstyle(FONTSTYLE_normal);
                    }
                    else {
                        // Explicitly set italic fontstyle.
                        item->SetFontstyle(FONTSTYLE_italic);
                    }
                    if (bold) {
                        item->SetFontweight(FONTWEIGHT_bold);
                    }
                    if (justification == 1) {
                        item->SetHalign(HORIZONTALALIGNMENT_right);
                    }
                    else if (justification == 2) {
                        item->SetHalign(HORIZONTALALIGNMENT_center);
                    }
                }
            }
        }
        else if (dir) {
            if (onlysmufl && needrend) {
                Rend *rend = new Rend();
                dir->AddChild(rend);
                addTextElement(rend, text);
                if (!color.empty()) {
                    rend->SetColor(color);
                }
                else if (problemQ) {
                    rend->SetColor("red");
                }
                else if (sicQ) {
                    rend->SetColor("limegreen");
                }
                if (!italic) {
                    rend->SetFontstyle(FONTSTYLE_normal);
                }
                else {
                    // Explicitly set italic fontstyle.
                    rend->SetFontstyle(FONTSTYLE_italic);
                }
                if (bold) {
                    rend->SetFontweight(FONTWEIGHT_bold);
                }
                if (justification == 1) {
                    rend->SetHalign(HORIZONTALALIGNMENT_right);
                }
                else if (justification == 2) {
                    rend->SetHalign(HORIZONTALALIGNMENT_center);
                }
            }
            else {
                addTextElement(dir, text);
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getMmTempo -- return any *MM# tempo value before or at the input token,
//     but before any data.
//     Returns 0.0 if no tempo is found.
//

double HumdrumInput::getMmTempo(hum::HTp token, bool checklast)
{
    hum::HumRegex hre;
    hum::HTp current = token;
    if (current && current->isData()) {
        current = current->getPreviousToken();
    }
    while (current && !current->isData()) {
        if (current->isInterpretation()) {
            if (hre.search(current, "^\\*MM(\\d+\\.?\\d*)")) {
                bool islast = isLastStaffTempo(current);
                if (!islast) {
                    return 0.0;
                }
                double tempo = hre.getMatchDouble(1);
                return tempo;
            }
        }
        current = current->getPreviousToken();
    }
    return 0.0;
}

//////////////////////////////
//
// HumdrumInput::getMmTempoForward -- return any *MM# tempo value before or at the input token,
//     but before any data.
//     Returns 0.0 if no tempo is found.
//

double HumdrumInput::getMmTempoForward(hum::HTp token)
{
    hum::HumRegex hre;
    hum::HTp current = token;
    if (current && current->isData()) {
        current = current->getNextToken();
    }
    int line = 0;
    while (current && current->getSpineInfo() == "") {
        line = current->getLineIndex() + 1;
        current = current->getOwner()->getOwner()->token(line, 0);
    }
    while (current && !current->isData()) {
        if (current->isInterpretation()) {
            if (hre.search(current, "^\\*MM(\\d+\\.?\\d*)")) {
                double tempo = hre.getMatchDouble(1);
                return tempo;
            }
        }
        current = current->getNextToken();
    }
    return 0.0;
}

//////////////////////////////
//
// HumdrumInput::addTempoDirection --
//
bool HumdrumInput::addTempoDirection(const std::string &text, const std::string &placement, bool bold, bool italic,
    hum::HTp token, int staffindex, int justification, const std::string &color)
{
    Tempo *tempo = new Tempo();
    double midibpm = getMmTempo(token);
    if (midibpm > 0) {
        tempo->SetMidiBpm(midibpm * m_globalTempoScaling * m_localTempoScaling.getFloat());
    }
    if (placement == "center") {
        setStaffBetween(tempo, m_currentstaff);
    }
    else {
        setStaff(tempo, m_currentstaff);
    }
    setLocationId(tempo, token);
    hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    if (token->isMensLike()) {
        // Attach to note, not with measure timestamp.
        // Need to handle text on chords (will currently have a problem attaching to chords)
        std::string startid = getLocationId("note", token);
        tempo->SetStartid("#" + startid);
    }
    else {
        tempo->SetTstamp(tstamp.getFloat());
    }

    if (placement == "above") {
        setPlaceRelStaff(tempo, "above", false);
    }
    else if (placement == "below") {
        setPlaceRelStaff(tempo, "below", false);
    }
    else if (placement == "center") {
        setPlaceRelStaff(tempo, "between", false);
    }

    // deal with rj and cj justification here

    bool status = setTempoContent(tempo, text);
    if (status) {
        addChildMeasureOrSection(tempo);
        return true;
    }
    else {
        addTextElement(tempo, text);
        addChildMeasureOrSection(tempo);
        return true;
    }
}

//////////////////////////////
//
// HumdrumInput::setLabelContent -- Will convert one Eb, E-flat, C#, C-sharp
//   encoded accidental into a VeorvioText (SMUFL) accidental.
//

bool HumdrumInput::setLabelContent(Label *label, const std::string &name)
{
    std::string name2 = name;
    std::string prestring;
    std::u32string symbol;
    std::string poststring;

    hum::HumRegex hre;
    if (hre.search(name2, "(.*)-flat\\b(.*)")) {
        prestring = hre.getMatch(1);
        poststring = hre.getMatch(2);
        symbol = U"\uE260"; // SMUFL flat
    }
    else if (hre.search(name2, "(.*)-sharp\\b(.*)")) {
        prestring = hre.getMatch(1);
        poststring = hre.getMatch(2);
        symbol = U"\uE262"; // SMUFL sharp
    }
    else if (hre.search(name2, "(.*\\b[A-G])b\\b(.*)")) {
        prestring = hre.getMatch(1);
        poststring = hre.getMatch(2);
        symbol = U"\uE260"; // SMUFL flat
    }
    else if (hre.search(name2, "(.*[A-G])\x23(.*)")) {
        prestring = hre.getMatch(1);
        poststring = hre.getMatch(2);
        symbol = U"\uE262"; // SMUFL sharp
    }

    if (symbol.empty()) {
        addTextElement(label, name2);
    }
    else {
        if (!prestring.empty()) {
            addTextElement(label, prestring);
        }
        Rend *rend = new Rend();
        Text *text = new Text();
        text->SetText(symbol);
        rend->AddChild(text);
        label->AddChild(rend);
        rend->SetFontfam("smufl");
        if (!poststring.empty()) {
            addTextElement(label, poststring);
        }
        // verovio probably eats the space surronding the
        // rend, so may need to force to be non-breaking space.
    }

    return true;
}

//////////////////////////////
//
// HumdrumInput::setTempoContent --
//

bool HumdrumInput::setTempoContent(Tempo *tempo, const std::string &text)
{
    hum::HumRegex hre;
    if (!hre.search(text, "(.*)\\[([^=\\]]*)\\]\\s*=\\s*(\\d+.*)")) {
        // no musical characters to unescape
        addTextElement(tempo, text);
        return true;
    }
    std::string first = hre.getMatch(1);
    std::string second = hre.getMatch(2);
    std::string third = hre.getMatch(3);
    std::vector<std::string> secondNames = convertMusicSymbolNameToSmuflName(second);

    if (!first.empty()) {
        if (first.back() == '(') {
            // Add very thin spacer after opening parenthesis
            // to separate parenthesis and notehead:
            first += "&#x200A;";
        }
        addTextElement(tempo, first);
    }

    // Add the musical symbols, adding a space between them
    std::string name;
    int counter = 0;
    for (int i = 0; i < (int)secondNames.size(); i++) {
        if (secondNames.at(i).empty()) {
            continue;
        }
        name = secondNames.at(i);
        if (counter) {
            // Add a space element between music symbols.
            if (name == "metAugmentationDot") {
                addTextElement(tempo, m_textAugmentationDotSpacer);
            }
            else {
                addTextElement(tempo, m_textSmuflSpacer);
            }
        }
        ++counter;

        Symbol *symbol = new Symbol();
        setSmuflContent(symbol, name);
        setFontsize(symbol, name, "");
        tempo->AddChild(symbol);
    }

    // Force spaces around equals sign:
    third = m_textSmuflSpacer + "=" + m_textSmuflSpacer + third;
    addTextElement(tempo, third);

    return true;
}

//////////////////////////////
//
// HumdrumInput::setSmuflContent -- Pull out any type parameter before inserting the name of the glyph.
//

void HumdrumInput::setSmuflContent(Symbol *symbol, const std::string &name)
{
    hum::HumRegex hre;
    std::string mytype;
    if (hre.search(name, "@type=\"(.*?)\"")) {
        mytype = hre.getMatch(1);
        std::string newname = hre.replaceCopy(name, "", "@.*");
        symbol->SetGlyphName(newname);
    }
    else {
        symbol->SetGlyphName(name);
    }

    symbol->SetGlyphAuth("smufl");

    if (!mytype.empty()) {
        symbol->SetType(mytype);
    }
}

//////////////////////////////
//
// HumdrumInput::setFontsize -- Set the fontsize of an element to a percentage or absolute size.
//

template <class ELEMENT>
void HumdrumInput::setFontsize(ELEMENT *element, const std::string &smuflname, const std::string &original)
{
    // Check for percentage override:
    hum::HumRegex hre;
    if (hre.search(original, "(\\d+\\.?\\d*%)")) {
        std::string perc = hre.getMatch(1);
        data_PERCENT fontpercent;
        fontpercent = ((vrv::AttTypography *)element)->StrToPercent(perc);
        data_FONTSIZE fontsize;
        fontsize.SetPercent(fontpercent);
        element->SetFontsize(fontsize);
        return;
    }

    // Check for explicit size override.  Verovio mappings to percent:
    //   xx_large => 200
    //   x_large  => 150
    //   large    => 110
    //   larger   => 110 (not used since same as large)
    //   small    => 80
    //   smaller  => 80 (not used since same as small)
    //   x_small  => 60
    //   xx_small => 50

    if (original.find("smaller") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_x_small);
        element->SetFontsize(fs);
        return;
    }
    else if (original.find("smallest") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_xx_small);
        element->SetFontsize(fs);
        return;
    }
    else if (original.find("small") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_small);
        element->SetFontsize(fs);
        return;
    }
    else if (original.find("larger") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_x_large);
        element->SetFontsize(fs);
        return;
    }
    else if (original.find("largest") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_xx_large);
        element->SetFontsize(fs);
        return;
    }
    else if (original.find("large") != std::string::npos) {
        data_FONTSIZE fs;
        fs.SetTerm(FONTSIZETERM_large);
        element->SetFontsize(fs);
        return;
    }

    if (smuflname.compare(0, 3, "met") == 0) {
        // If smuflname is for a metric note, then shrink the size to 70% by default.
        data_PERCENT fontpercent;
        fontpercent = ((vrv::AttTypography *)element)->StrToPercent(m_textNoteSize);
        data_FONTSIZE fontsize;
        fontsize.SetPercent(fontpercent);
        element->SetFontsize(fontsize);
        return;
    }
}

//////////////////////////////
//
// HumdrumInput::convertMusicSymbolNameToSmuflName --  Convert from
//    text names for music symbols into SMuFL names for display with <symbol>
//
//    Humdrum name                 SMuFL name
// ===============================================================
//    segno                        segno
//    coda                         coda
//    [Pp]ed                       pianoPedalPed
//    [Xx][Pp]ed                   pianoPedalPedUp
//    sc                           mensuralSignumUp
//    sc-below                     mensuralSignumDown
//    breve                        metNoteDoubleWholeSquare
//    whole                        metNoteWhole
//    half                         metNoteHalfUp
//    quarter                      metNoteQuarterUp
//    eighth                       metNote8thUp
//    sixteenth                    metNote16thUp
//    32nd                         metNote32ndUp
//    64th                         metNote64thUp
//    128th                        metNote128thUp
//    256th                        metNote256thUp
//    512th                        metNote512thUp
//    1024th                       metNote1024thUp
//    -dot                         metAugmentationDot
//    circle-dot, [Oo]-dot         mensuralProlation1
//    circle, [Oo]                 mensuralProlation2
//    cut-circle, cut-[Oo]         mensuralProlation3
//    cut-circle-dot, cut-[Oo]-dot mensuralProlation4
//    [Cc]-dot                     mensuralProlation5
//    C                            mensuralProlation6
//    c                            timeSigCommon
//    reverse-[Cc], [Cc]r          mensuralProlation7
//    cut-[Cc]-dot                 mensuralProlation8
//    cut-C                        mensuralProlation9
//    cut-c                        timeSigCutCommon
//    reverse-cut-[Cc]c            mensuralProlation10
//    reverse-[Cc]-dot             mensuralProlation11
//    [Oo]-slash                   mensuralProportionTempusPerfectum
//

std::vector<std::string> HumdrumInput::convertMusicSymbolNameToSmuflName(const std::string &text)
{
    std::vector<std::string> output;

    if (text.empty()) {
        return output;
    }

    std::string newtext;
    if ((text[0] == '[') && (text.back() == ']')) {
        newtext = text.substr(1, text.size() - 2);
    }
    else {
        newtext = text;
    }

    // Remove styling qualifiers:
    hum::HumRegex hre;
    std::string finaltext = newtext;
    hre.replaceDestructive(finaltext, "", "[|@].*");

    // https://www.smufl.org/version/latest/range/repeats

    if (finaltext == "segno") {
        output.push_back("segno");
        return output;
    }
    if (finaltext == "coda") {
        output.push_back("coda");
        return output;
    }

    // https://www.smufl.org/version/1.2/range/keyboardTechniques

    if ((finaltext == "Ped") || (finaltext == "ped")) {
        output.push_back("keyboardPedalPed");
        return output;
    }

    if ((finaltext == "XPed") || (finaltext == "xped")) {
        output.push_back("keyboardPedalPedUp");
        return output;
    }

    // http://www.smufl.org/version/1.2/range/medievalAndRenaissanceMiscellany

    if (finaltext == "sc") {
        output.push_back("mensuralSignumUp");
        return output;
    }
    if (finaltext == "sc-below") {
        output.push_back("mensuralSignumDown");
        return output;
    }

    // http://www.smufl.org/version/1.2/range/medievalAndRenaissanceProlations

    if ((finaltext == "circle-dot") || (finaltext == "o-dot") || (finaltext == "O-dot")) {
        output.push_back("mensuralProlation1@type=\"circle-dot\"");
        return output;
    }
    if ((finaltext == "circle") || (finaltext == "O") || (finaltext == "o")) {
        output.push_back("mensuralProlation2@type=\"circle\"");
        return output;
    }
    if ((finaltext == "cut-circle") || (finaltext == "cut-o") || (finaltext == "cut-O")) {
        output.push_back("mensuralProlation3@type=\"cut-circle\"");
        return output;
    }
    if ((finaltext == "cut-circle-dot") || (finaltext == "cut-o-dot") || (finaltext == "cut-O-dot")
        || (finaltext == "O.!")) {
        output.push_back("mensuralProlation4@type=\"cut-circle-dot\"");
        return output;
    }
    if ((finaltext == "c-dot") || (finaltext == "C-dot")) {
        output.push_back("mensuralProlation5@type=\"c-dot\"");
        return output;
    }
    if (finaltext == "C") {
        output.push_back("mensuralProlation6@type=\"c\"");
        return output;
    }
    if (finaltext == "c") {
        output.push_back("timeSigCommon");
        return output;
    }
    if ((finaltext == "reverse-c") || (finaltext == "Cr") || (finaltext == "cr")) {
        output.push_back("mensuralProlation7@type=\"reverse-c\"");
        return output;
    }
    if ((finaltext == "cut-c-dot") || (finaltext == "cut-C-dot")) {
        output.push_back("mensuralProlation8@type=\"cut-c-dot\"");
        return output;
    }
    if (finaltext == "cut-C") {
        output.push_back("mensuralProlation9@type=\"cut-c\"");
        return output;
    }
    if (finaltext == "cut-c") {
        output.push_back("timeSigCutCommon");
        return output;
    }

    if ((finaltext == "reverse-cut-c") || (finaltext == "reverse-cut-C") || (finaltext == "cut-cr")
        || (finaltext == "cut-Cr")) {
        output.push_back("mensuralProlation10@type=\"reverse-cut-c\"");
        return output;
    }
    if ((finaltext == "reverse-c-dot") || (finaltext == "reverse-C-dot") || (finaltext == "cr-dot")
        || (finaltext == "Cr-dot")) {
        output.push_back("mensuralProlation11@type=\"reverse-c-dot\"");
        return output;
    }
    if ((finaltext == "circle-slash") || (finaltext == "o-slash") || (finaltext == "O/") || (finaltext == "o/")) {
        output.push_back("mensuralProportionTempusPerfectum");
        return output;
    }

    // https://www.smufl.org/version/1.2/range/metronomeMarks

    // Count the number of augmentation dots on the note, and then remove them:
    int dots = 0;
    if (hre.search(finaltext, "-dot$")) {
        dots = 1;
        if (hre.search(finaltext, "-dot-dot$")) {
            dots = 2;
            if (hre.search(finaltext, "-dot-dot-dot$")) {
                dots = 3;
            }
            // Only allowing three augmentation dots.
        }
        hre.replaceDestructive(finaltext, "", "(-dot)+");
    }
    // Check for "." used as an augmentation dot (typically used with numbers):
    if (hre.search(finaltext, "(\\.+)$")) {
        std::string dotstring = hre.getMatch(1);
        dots += (int)dotstring.size();
        hre.replaceDestructive(finaltext, "", "\\.+$");
    }

    bool noteQ = false;
    if ((finaltext == "quarter") || (finaltext == "4")) {
        output.push_back("metNoteQuarterUp");
        noteQ = true;
    }
    else if ((finaltext == "half") || (finaltext == "2")) {
        output.push_back("metNoteHalfUp");
        noteQ = true;
    }
    else if ((finaltext == "whole") || (finaltext == "1")) {
        output.push_back("metNoteWhole");
        noteQ = true;
    }
    else if ((finaltext == "breve") || (finaltext == "double-whole") || (finaltext == "0")) {
        output.push_back("metNoteSquareBreve");
        noteQ = true;
    }
    else if ((finaltext == "eighth") || (finaltext == "8") || (finaltext == "8th")) {
        output.push_back("metNote8thUp");
        noteQ = true;
    }
    else if ((finaltext == "sixteenth") || (finaltext == "16") || (finaltext == "16th")) {
        output.push_back("metNote16thUp");
        noteQ = true;
    }
    else if ((finaltext == "32") || (finaltext == "32nd")) {
        output.push_back("metNote32ndUp");
        noteQ = true;
    }
    else if ((finaltext == "64") || (finaltext == "64th")) {
        output.push_back("metNote64thUp");
        noteQ = true;
    }
    else if ((finaltext == "128") || (finaltext == "128th")) {
        output.push_back("metNote128thUp");
        noteQ = true;
    }
    else if ((finaltext == "256") || (finaltext == "256th")) {
        output.push_back("metNote256thUp");
        noteQ = true;
    }
    else if ((finaltext == "512") || (finaltext == "512th")) {
        output.push_back("metNote512thUp");
        noteQ = true;
    }
    else if ((finaltext == "1024") || (finaltext == "1024th")) {
        output.push_back("metNote1024thUp");
        noteQ = true;
    }

    if (dots) {
        for (int i = 0; i < dots; i++) {
            output.push_back("metAugmentationDot");
        }
    }

    if (noteQ) {
        return output;
    }

    // Getting here means that the symbol is unknown, and should be
    // as plain text elsewhere.  If the form is [smufl=code], then treat
    // this as an escape to display a particular SMuFL named glyph.
    if (hre.search(finaltext, "smufl=(.*)")) {
        output.push_back(hre.getMatch(1));
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::addDirection --
//     default value: color = "";
//
//     token->getLayoutParameter() should not be used in this function.  Instead
//     paste the parameter set that generate a text direction (there could be multiple
//     text directions attached to the note, and using getPayoutParameter() will merge
//     all of their parameters incorrectly.
//

void HumdrumInput::addDirection(const std::string &text, const std::string &placement, bool bold, bool italic,
    hum::HTp token, int staffindex, int justification, const std::string &color, int vgroup)
{
    hum::HumRegex hre;
    if (hre.search(text, "\\[[^=]*\\]\\s*=\\s*\\d+")) {
        int status = addTempoDirection(text, placement, bold, italic, token, staffindex, justification, color);
        if (status) {
            return;
        }
    }
    if (token->isTimeSignature()) {
        addTempoDirection(text, placement, bold, italic, token, staffindex, justification, color);
        return;
    }

    Dir *dir = new Dir();
    if (placement == "center") {
        setStaffBetween(dir, m_currentstaff);
    }
    else {
        setStaff(dir, m_currentstaff);
    }
    setLocationId(dir, token);
    hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    if (token->isMensLike()) {
        // Attach to note, not with measure timestamp.
        // Need to handle text on chords (will currently have a problem attaching to chords)
        std::string startid = getLocationId("note", token);
        dir->SetStartid("#" + startid);
    }
    else {
        dir->SetTstamp(tstamp.getFloat());
    }

    if (vgroup > 0) {
        dir->SetVgrp(vgroup);
    }

    // convert to HPS input value in the future:
    bool problemQ = false;
    std::string problem = token->getLayoutParameter("TX", "problem");
    if (problem == "true") {
        problemQ = true;
        appendTypeTag(dir, "problem");
    }

    bool sicQ = false;
    std::string sic = token->getLayoutParameter("SIC", "sic");
    if (sic == "true") {
        sicQ = true;
        appendTypeTag(dir, "sic");
    }

    // convert to HPS input value in the future:
    std::string typevalue = token->getLayoutParameter("TX", "type");
    if (!typevalue.empty()) {
        appendTypeTag(dir, typevalue);
    }

    addChildMeasureOrSection(dir);
    if (placement == "above") {
        setPlaceRelStaff(dir, "above", false);
    }
    else if (placement == "below") {
        setPlaceRelStaff(dir, "below", false);
    }
    else if (placement == "center") {
        setPlaceRelStaff(dir, "between", false);
    }
    bool plain = !(italic || bold);
    bool needrend = plain || bold || justification || color.size();
    if (needrend) {
        Rend *rend = new Rend();
        if (!color.empty()) {
            rend->SetColor(color);
        }
        else if (problemQ) {
            rend->SetColor("red");
        }
        else if (sicQ) {
            rend->SetColor("limegreen");
        }
        dir->AddChild(rend);
        addTextElement(rend, text);
        if (!italic) {
            rend->SetFontstyle(FONTSTYLE_normal);
        }
        else {
            // Explicitly set italic fontstyle.
            rend->SetFontstyle(FONTSTYLE_italic);
        }
        if (bold) {
            rend->SetFontweight(FONTWEIGHT_bold);
        }
        if (justification == 1) {
            rend->SetHalign(HORIZONTALALIGNMENT_right);
        }
        else if (justification == 2) {
            rend->SetHalign(HORIZONTALALIGNMENT_center);
        }
    }
    else {
        addTextElement(dir, text);
    }
}

/////////////////////////////
//
// HumdrumInput::processDynamics --
//

void HumdrumInput::processDynamics(hum::HTp token, int staffindex)
{
    std::string tok;
    std::string dynamic;
    bool graceQ = token->isGrace();
    hum::HumdrumLine *line = token->getLine();
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    if (line == NULL) {
        return;
    }
    int track = token->getTrack();
    int lasttrack = track;
    int ttrack;
    int startfield = token->getFieldIndex() + 1;

    bool forceAboveQ = false;
    bool forceBelowQ = false;
    bool forceCenterQ = false;
    // int forcebelowadj = 0;
    // int forceaboveadj = 0;
    int trackdiff = 0;
    int staffadj = ss[staffindex].m_dynamstaffadj;
    int forceQ = false;

    if (ss[staffindex].m_dynampos > 0) {
        forceQ = true;
        forceAboveQ = true;
        // forceaboveadj = -(ss[staffindex].m_dynampos - 1);
    }
    else if (ss[staffindex].m_dynampos < 0) {
        forceQ = true;
        forceBelowQ = true;
        // forcebelowadj = -ss[staffindex].m_dynamstaffadj;
    }
    else if ((ss[staffindex].m_dynampos == 0) && (ss[staffindex].m_dynamposdefined)) {
        forceCenterQ = true;
    }
    else if (ss[staffindex].verse) {
        forceAboveQ = true;
    }

    int justification = 0;
    if (token->getLayoutParameter("DY", "rj") == "true") {
        justification = 1;
    }
    if (token->getLayoutParameter("DY", "cj") == "true") {
        justification = 2;
    }

    std::string dcolor = token->getLayoutParameter("DY", "color");

    bool needsrend = justification || dcolor.size();

    // Handle "z" for sforzando (sf), or "zz" for sfz:

    bool aboveQ = false;
    bool belowQ = false;
    bool centerQ = false;
    bool showpos = false;

    auto loc = token->rfind("z");
    if (token->find("zy") != std::string::npos) {
        // don't show invisible sfz.
        // do nothing
    }
    else if (loc != std::string::npos) {
        int subtrack = token->getSubtrack();
        switch (subtrack) {
            case 1:
                aboveQ = true;
                belowQ = false;
                centerQ = false;
                break;
            case 2:
                belowQ = true;
                aboveQ = false;
                centerQ = false;
                break;
        }
        if (hasAboveParameter(token, "DY", staffadj)) {
            aboveQ = true;
            belowQ = false;
            centerQ = false;
            showpos = true;
        }
        if (!aboveQ) {
            if (hasBelowParameter(token, "DY", staffadj)) {
                aboveQ = false;
                belowQ = true;
                centerQ = false;
                showpos = true;
                if (belowQ && staffadj) {
                    staffadj--;
                }
                else if (forceQ && forceBelowQ) {
                    staffadj = -ss[staffindex].m_dynamstaffadj;
                }
            }
        }
        if (!aboveQ && !belowQ) {
            if (hasCenterParameter(token, "DY", staffadj)) {
                aboveQ = false;
                belowQ = false;
                centerQ = true;
                showpos = true;
            }
        }

        // This code block should probably be deleted.
        if (m_signifiers.below && (loc < token->size() - 1) && (token->at(loc + 1) == m_signifiers.below)) {
            aboveQ = false;
            belowQ = true;
            showpos = true;
        }
        if (m_signifiers.above && (loc < token->size() - 1) && (token->at(loc + 1) == m_signifiers.above)) {
            aboveQ = true;
            belowQ = false;
            showpos = true;
        }

        Dynam *dynam = new Dynam();
        addChildMeasureOrSection(dynam);

        int newstaff = m_currentstaff + staffadj;
        if (newstaff < 1) {
            newstaff = 1;
        }
        if (newstaff > (int)ss.size()) {
            newstaff = (int)ss.size();
        }
        setStaff(dynam, newstaff);

        if (needsrend) {
            Rend *rend = new Rend();
            dynam->AddChild(rend);
            rend->SetFontweight(rend->AttTypography::StrToFontweight("bold"));
            data_FONTSIZE fs;
            fs.SetTerm(FONTSIZETERM_large);
            rend->SetFontsize(fs);
            if (token->find("zz") != std::string::npos) {
                addTextElement(rend, "sfz&#160;");
            }
            else {
                addTextElement(rend, "sf&#160;");
            }
            if (!dcolor.empty()) {
                rend->SetColor(dcolor);
            }
            if (justification == 1) {
                rend->SetHalign(HORIZONTALALIGNMENT_right);
            }
            else if (justification == 2) {
                rend->SetHalign(HORIZONTALALIGNMENT_center);
            }
        }
        else {
            if (token->find("zz") != std::string::npos) {
                addTextElement(dynam, "sfz");
            }
            else {
                addTextElement(dynam, "sf");
            }
        }

        setLocationId(dynam, token, -1);
        hum::HumNum barstamp = getMeasureTstamp(token, staffindex);
        dynam->SetTstamp(barstamp.getFloat());

        if (aboveQ) {
            setPlaceRelStaff(dynam, "above", showpos);
        }
        else if (belowQ) {
            setPlaceRelStaff(dynam, "below", showpos);
        }
        else if (centerQ) {
            setPlaceRelStaff(dynam, "between", showpos);
        }
        else if (forceAboveQ) {
            setPlaceRelStaff(dynam, "above", showpos);
        }
        else if (forceBelowQ) {
            setPlaceRelStaff(dynam, "below", showpos);
        }
        else if (forceCenterQ) {
            setPlaceRelStaff(dynam, "between", showpos);
        }
    }

    bool active = true;
    for (int i = startfield; i < line->getFieldCount(); ++i) {
        staffadj = ss[staffindex].m_dynamstaffadj;
        hum::HTp dyntok = line->token(i);
        std::string exinterp = dyntok->getDataType();
        if ((exinterp != "**kern") && (exinterp.find("kern") != std::string::npos)) {
            active = false;
        }
        if (dyntok->isKernLike()) {
            active = true;
            ttrack = dyntok->getTrack();
            if (ttrack != track) {
                if (ttrack != lasttrack) {
                    trackdiff++;
                    lasttrack = ttrack;
                }
                if (graceQ) {
                    continue;
                }
                else {
                    break;
                }
            }
            // Break if this is not the last layer for the current spine
            if (!dyntok->isNull()) {
                break;
            }
        }
        if (!active) {
            continue;
        }
        if (!(dyntok->isDataType("**dynam") || dyntok->isDataType("**dyn"))) {
            continue;
        }
        // Don't skip NULL tokens, because this algorithm only prints dynamics
        // after the last layer, and there could be notes in earlier layer
        // that need the dynamic.
        // if (dyntok->isNull()) {
        //     continue;
        // }

        std::string tok = *line->token(i);
        if (dyntok->getValueBool("auto", "DY", "processed")) {
            return;
        }
        dyntok->setValue("auto", "DY", "processed", "true");

        // int pcount = dyntok->getLinkedParameterSetCount();

        std::string hairpins;
        std::string letters;
        for (int i = 0; i < (int)tok.size(); ++i) {
            if (isalpha(tok[i])) {
                letters.push_back(tok[i]);
            }
            else {
                hairpins.push_back(tok[i]);
            }
        }
        hum::HumRegex hre;

        if (hre.search(letters, "^[sr]?f+z?$")) {
            dynamic = letters;
        }
        else if (hre.search(letters, "^p+$")) {
            dynamic = letters;
        }
        else if (hre.search(letters, "^m?(f|p)$")) {
            dynamic = letters;
        }
        else if (hre.search(letters, "^s?f+z?p+$")) {
            dynamic = letters;
        }
        if (!dynamic.empty()) {
            staffadj = ss[staffindex].m_dynamstaffadj;

            std::string dyntext = getLayoutParameter(dyntok, "DY", "t", "", "");
            if (!dyntext.empty()) {
                hum::HumRegex hre;
                hre.replaceDestructive(dyntext, dynamic, "%s", "g");
                dynamic = dyntext;
            }

            bool aboveQ = hasAboveParameter(dyntok, "DY", staffadj);
            bool belowQ = false;
            bool showplace = aboveQ;
            if (!aboveQ) {
                belowQ = hasBelowParameter(dyntok, "DY", staffadj);
                showplace = belowQ;
            }
            if (!aboveQ && !belowQ) {
                if (hasCenterParameter(dyntok, "DY", staffadj)) {
                    aboveQ = false;
                    belowQ = false;
                    centerQ = true;
                    showplace = centerQ;
                }
            }

            // if pcount > 0, then search for prefix and postfix text
            // to add to the dynamic.
            // std::string prefix = "aaa ";
            // std::string postfix = " bbb";
            // See https://github.com/music-encoding/music-encoding/issues/540

            int justification = 0;
            if (dyntok->getLayoutParameter("DY", "rj") == "true") {
                justification = 1;
            }
            else if (dyntok->getLayoutParameter("DY", "cj") == "true") {
                justification = 2;
            }

            bool editQ = false;
            bool brackQ = false;
            bool parenQ = false;
            bool curlyQ = false;
            bool angleQ = false;

            std::string editstr = getLayoutParameter(dyntok, "DY", "ed", "true");
            if (editstr == "true") {
                editQ = true;
            }
            if (editstr.find("brack") != std::string::npos) {
                brackQ = true;
                editQ = true;
            }
            else if (editstr.find("paren") != std::string::npos) {
                parenQ = true;
                editQ = true;
            }
            else if (editstr.find("curly") != std::string::npos) {
                curlyQ = true;
                std::string newdynamic = "{ ";
                newdynamic += dynamic;
                newdynamic += " }";
                dynamic = newdynamic;
            }
            else if (editstr.find("angle") != std::string::npos) {
                angleQ = true;
                std::string newdynamic = "< ";
                newdynamic += dynamic;
                newdynamic += " >";
                dynamic = newdynamic;
            }
            if (!(parenQ || brackQ || curlyQ || angleQ)) {
                std::string parenP = getLayoutParameter(dyntok, "DY", "paren", "true");
                std::string brackP = getLayoutParameter(dyntok, "DY", "brack", "true");
                std::string curlyP = getLayoutParameter(dyntok, "DY", "curly", "true");
                std::string angleP = getLayoutParameter(dyntok, "DY", "angle", "true");
                if (parenP == "true") {
                    parenQ = true;
                }
                else if (brackP == "true") {
                    brackQ = true;
                }
                else if (curlyP == "true") {
                    curlyQ = true;
                    std::string newdynamic = "{ ";
                    newdynamic += dynamic;
                    newdynamic += " }";
                    dynamic = newdynamic;
                }
                else if (angleP == "true") {
                    angleQ = true;
                    std::string newdynamic = "< ";
                    newdynamic += dynamic;
                    newdynamic += " >";
                    dynamic = newdynamic;
                }
            }

            std::string dcolor = dyntok->getLayoutParameter("DY", "color");
            int needsrend = justification || dcolor.size();

            Dynam *dynam = new Dynam();
            if (editQ) {
                Supplied *supplied = new Supplied();
                appendElement(supplied, dynam);
                addChildMeasureOrSection(supplied);
                appendTypeTag(dynam, "editorial");
            }
            else {
                addChildMeasureOrSection(dynam);
            }

            if (parenQ) {
                dynam->SetEnclose(ENCLOSURE_paren);
            }
            if (brackQ) {
                dynam->SetEnclose(ENCLOSURE_brack);
            }

            int newstaff = m_currentstaff - staffadj;
            if (newstaff < 1) {
                newstaff = 1;
            }
            if (newstaff > (int)ss.size()) {
                newstaff = (int)ss.size();
            }

            if ((centerQ || forceCenterQ) && !aboveQ && !belowQ) {
                setStaffBetween(dynam, newstaff);
            }
            else {
                setStaff(dynam, newstaff);
            }
            setLocationId(dynam, dyntok, -1);

            if (needsrend) {
                Rend *rend = new Rend();
                dynam->AddChild(rend);
                rend->SetFontweight(rend->AttTypography::StrToFontweight("bold"));
                data_FONTSIZE fs;
                fs.SetTerm(FONTSIZETERM_large);
                rend->SetFontsize(fs);
                // addTextElement(rend, prefix);
                std::string newtext = dynamic + "&#160;";
                addTextElement(rend, newtext);
                // addTextElement(rend, postfix);
                if (!dcolor.empty()) {
                    rend->SetColor(dcolor);
                }
                if (justification == 1) {
                    rend->SetHalign(HORIZONTALALIGNMENT_right);
                }
                else if (justification == 2) {
                    rend->SetHalign(HORIZONTALALIGNMENT_center);
                }
            }
            else {
                // addTextElement(dynam, prefix);
                addTextElement(dynam, dynamic);
                // addTextElement(dynam, postfix);
            }

            hum::HumNum linedur = line->getDuration();
            if (linedur == 0) {
                // Grace note line, so attach to the note rather than
                // the timestamp.
                if (token->isChord()) {
                    dynam->SetStartid("#" + getLocationId("chord", token));
                }
                else {
                    // maybe check if a null token instead of a note here.
                    dynam->SetStartid("#" + getLocationId("note", token));
                }
            }
            else {
                hum::HumNum barstamp = getMeasureTstamp(token, staffindex);
                dynam->SetTstamp(barstamp.getFloat());
            }

            std::string verticalgroup = dyntok->getLayoutParameter("DY", "vg");
            if (!forceCenterQ) {
                // Not allowing vertical groups for "between" placements for now.
                if (verticalgroup.empty()) {
                    // 100 is the default group for dynamics:
                    dynam->SetVgrp(VGRP_DYNAM_DEFAULT);
                }
                else if (std::isdigit(verticalgroup[0])) {
                    dynam->SetVgrp(stoi(verticalgroup));
                }
                else {
                    // don't set a vertical group for this token
                }
            }

            if (trackdiff == 1) {
                // case needed for grace notes in the bottom staff of a grand staff.
                setPlaceRelStaff(dynam, "above", false);
            }
            if (aboveQ) {
                setPlaceRelStaff(dynam, "above", showplace);
            }
            else if (belowQ) {
                setPlaceRelStaff(dynam, "below", showplace);
            }
            else if (centerQ) {
                setPlaceRelStaff(dynam, "between", showplace);
            }
            else if (forceAboveQ) {
                setPlaceRelStaff(dynam, "above", false);
            }
            else if (forceBelowQ) {
                setPlaceRelStaff(dynam, "below", false);
            }
            else if (forceCenterQ) {
                setPlaceRelStaff(dynam, "between", false);
            }
        }
        if (hairpins.find("<") != std::string::npos) {
            int endline = false;
            hum::HTp endtok = NULL;
            hum::HumNum duration = 0;
            if ((hairpins.find("<[") != std::string::npos) || (hairpins.find("< [") != std::string::npos)) {
                duration = getLeftNoteDuration(token);
                endtok = token;
                endline = true;
            }
            else {
                endtok = getCrescendoEnd(dyntok);
            }
            staffadj = ss[staffindex].m_dynamstaffadj;
            bool aboveQ = hasAboveParameter(dyntok, "HP", staffadj);
            bool belowQ = false;
            bool centerQ = false;
            bool showplace = aboveQ;
            if (!aboveQ) {
                belowQ = hasBelowParameter(dyntok, "HP", staffadj);
                showplace = belowQ;
            }
            if (!aboveQ && !belowQ) {
                if (hasCenterParameter(dyntok, "HP", staffadj)) {
                    aboveQ = false;
                    belowQ = false;
                    centerQ = true;
                    showplace = centerQ;
                }
            }

            if (endtok != NULL) {
                Hairpin *hairpin = new Hairpin();
                int newstaff = m_currentstaff - staffadj;
                if (newstaff < 1) {
                    newstaff = 1;
                }
                if (newstaff > (int)ss.size()) {
                    newstaff = (int)ss.size();
                }
                if ((centerQ || forceCenterQ) && !aboveQ && !belowQ) {
                    setStaffBetween(hairpin, newstaff);
                    setPlaceRelStaff(hairpin, "between", showplace);
                }
                else {
                    setStaff(hairpin, newstaff);
                    if (aboveQ) {
                        setPlaceRelStaff(hairpin, "above", showplace);
                    }
                    else if (belowQ) {
                        setPlaceRelStaff(hairpin, "below", showplace);
                    }
                    else if (forceAboveQ) {
                        setPlaceRelStaff(hairpin, "above", showplace);
                    }
                    else if (forceBelowQ) {
                        setPlaceRelStaff(hairpin, "below", showplace);
                    }
                }
                setLocationId(hairpin, dyntok, -1);
                std::string color = getLoColor(dyntok, "HP");
                if (!color.empty()) {
                    hairpin->SetColor(color);
                }
                hum::HumNum tstamp = getMeasureTstamp(dyntok, staffindex);
                hum::HumNum tstamp2;
                if (duration > 0) {
                    tstamp2 = getMeasureTstamp(dyntok, duration, staffindex);
                }
                else {
                    tstamp2 = getMeasureTstamp(endtok, staffindex);
                }
                if ((duration == 0) && (endline || (endtok->find("[[") != std::string::npos))) {
                    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
                    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
                    tstamp2 += endtok->getLine()->getDuration() * mfactor;
                }
                int measures = getMeasureDifference(dyntok, endtok);

                setAttachmentType(hairpin, token);
                pair<int, double> ts2(measures, tstamp2.getFloat());
                hairpin->SetTstamp2(ts2);
                hairpin->SetForm(hairpinLog_FORM_cres);
                addChildMeasureOrSection(hairpin);

                std::string verticalgroup = dyntok->getLayoutParameter("HP", "vg");
                if (!forceCenterQ) {
                    // Not allowing vertical groups for "between" placements for now.
                    if (verticalgroup.empty()) {
                        // 100 is the default group for dynamics:
                        hairpin->SetVgrp(VGRP_DYNAM_DEFAULT);
                    }
                    else if (std::isdigit(verticalgroup[0])) {
                        hairpin->SetVgrp(stoi(verticalgroup));
                    }
                    else {
                        // don't set a vertical group for this token
                    }
                }
            }
            else {
                // no endpoint so print as the word "cresc."
                Dir *dir = new Dir();
                addChildMeasureOrSection(dir);
                int newstaff = m_currentstaff - staffadj;
                if (newstaff < 1) {
                    newstaff = 1;
                }
                else if (newstaff > (int)ss.size()) {
                    newstaff = (int)ss.size();
                }
                if ((centerQ || forceCenterQ) && !aboveQ && !belowQ) {
                    setStaffBetween(dir, newstaff);
                    setPlaceRelStaff(dir, "between", showplace);
                }
                else {
                    setStaff(dir, newstaff);

                    if (aboveQ) {
                        setPlaceRelStaff(dir, "above", showplace);
                    }
                    else if (belowQ) {
                        setPlaceRelStaff(dir, "below", showplace);
                    }
                    else if (forceAboveQ) {
                        setPlaceRelStaff(dir, "above", false);
                    }
                    else if (forceBelowQ) {
                        setPlaceRelStaff(dir, "below", false);
                    }
                }
                setLocationId(dir, dyntok);
                setAttachmentType(dir, token);

                std::string fontstyle;
                std::string content = "cresc.";

                if (!m_signifiers.cresctext.empty()) {
                    content = m_signifiers.cresctext;
                    fontstyle = m_signifiers.crescfontstyle;
                }

                std::string pintext = getLayoutParameter(dyntok, "HP", "t", "", "");
                if (!pintext.empty()) {
                    hum::HumRegex hre;
                    hre.replaceDestructive(pintext, content, "%s", "g");
                    content = pintext;
                }

                addTextElement(dir, content, fontstyle);
            }
        }
        else if (hairpins.find(">") != std::string::npos) {
            int endline = false;
            hum::HTp endtok = NULL;
            hum::HumNum duration = 0;
            if ((hairpins.find(">]") != std::string::npos) || (hairpins.find("> ]") != std::string::npos)) {
                duration = getLeftNoteDuration(token);
                endtok = token;
                endline = true;
            }
            else {
                endtok = getDecrescendoEnd(dyntok);
            }

            staffadj = ss[staffindex].m_dynamstaffadj;
            bool aboveQ = hasAboveParameter(dyntok, "HP", staffadj);
            bool belowQ = false;
            bool centerQ = false;
            bool showplace = aboveQ;
            if (!aboveQ) {
                belowQ = hasBelowParameter(dyntok, "HP", staffadj);
                showplace = belowQ;
            }
            if (!aboveQ && !belowQ) {
                centerQ = hasCenterParameter(dyntok, "HP", staffadj);
                showplace = centerQ;
            }

            if (endtok != NULL) {
                Hairpin *hairpin = new Hairpin();
                int newstaff = m_currentstaff - staffadj;

                if (newstaff < 1) {
                    newstaff = 1;
                }
                else if (newstaff > (int)ss.size()) {
                    newstaff = (int)ss.size();
                }
                if ((centerQ || forceCenterQ) && !aboveQ && !belowQ) {
                    setStaffBetween(hairpin, newstaff);
                    setPlaceRelStaff(hairpin, "between", showplace);
                }
                else {
                    setStaff(hairpin, newstaff);
                    if (aboveQ) {
                        setPlaceRelStaff(hairpin, "above", showplace);
                    }
                    else if (belowQ) {
                        setPlaceRelStaff(hairpin, "below", showplace);
                    }
                    else if (forceAboveQ) {
                        setPlaceRelStaff(hairpin, "above", showplace);
                    }
                    else if (forceBelowQ) {
                        setPlaceRelStaff(hairpin, "below", showplace);
                    }
                }
                std::string color = getLoColor(dyntok, "HP");
                if (!color.empty()) {
                    hairpin->SetColor(color);
                }
                setLocationId(hairpin, dyntok, -1);
                hum::HumNum tstamp = getMeasureTstamp(dyntok, staffindex);
                hum::HumNum tstamp2;
                if (duration > 0) {
                    tstamp2 = getMeasureTstamp(dyntok, duration, staffindex);
                }
                else {
                    tstamp2 = getMeasureTstamp(endtok, staffindex);
                }
                if ((duration == 0) && (endline || (endtok->find("]]") != std::string::npos))) {
                    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
                    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
                    tstamp2 += endtok->getLine()->getDuration() * mfactor;
                }
                int measures = getMeasureDifference(dyntok, endtok);

                setAttachmentType(hairpin, token);

                pair<int, double> ts2(measures, tstamp2.getFloat());
                hairpin->SetTstamp2(ts2);
                hairpin->SetForm(hairpinLog_FORM_dim);

                addChildMeasureOrSection(hairpin);

                std::string verticalgroup = dyntok->getLayoutParameter("HP", "vg");
                if (verticalgroup.empty()) {
                    // 100 is the default group for dynamics:
                    hairpin->SetVgrp(VGRP_DYNAM_DEFAULT);
                }
                else if (std::isdigit(verticalgroup[0])) {
                    hairpin->SetVgrp(stoi(verticalgroup));
                }
                else {
                    // don't set a vertical group for this token
                }
            }
            else {
                // no endpoint so print as the word "decresc."
                Dir *dir = new Dir();
                addChildMeasureOrSection(dir);
                int staffadj = ss[staffindex].m_dynamstaffadj;
                bool aboveQ = hasAboveParameter(dyntok, "HP", staffadj);
                bool belowQ = false;
                bool centerQ = false;
                if (!aboveQ) {
                    belowQ = hasBelowParameter(dyntok, "HP", staffadj);
                }
                if (!aboveQ && !belowQ) {
                    centerQ = hasCenterParameter(dyntok, "HP", staffadj);
                }

                int newstaff = m_currentstaff - staffadj;
                if (newstaff < 1) {
                    newstaff = 1;
                }
                else if (newstaff > (int)ss.size()) {
                    newstaff = (int)ss.size();
                }

                if ((centerQ || forceCenterQ) && !aboveQ && !belowQ) {
                    setStaffBetween(dir, newstaff);
                    setPlaceRelStaff(dir, "between", showplace);
                }
                else {
                    setStaff(dir, newstaff);

                    if (aboveQ) {
                        setPlaceRelStaff(dir, "above", showplace);
                    }
                    else if (belowQ) {
                        setPlaceRelStaff(dir, "below", showplace);
                    }
                    else if (forceAboveQ) {
                        setPlaceRelStaff(dir, "above", showplace);
                    }
                    else if (forceBelowQ) {
                        setPlaceRelStaff(dir, "below", showplace);
                    }
                }

                setLocationId(dir, dyntok);
                setAttachmentType(dir, token);

                std::string fontstyle = "";
                std::string content = "decresc.";
                if (!m_signifiers.decresctext.empty()) {
                    content = m_signifiers.decresctext;
                    fontstyle = m_signifiers.decrescfontstyle;
                }

                std::string pintext = getLayoutParameter(dyntok, "HP", "t", "", "");
                if (!pintext.empty()) {
                    hum::HumRegex hre;
                    hre.replaceDestructive(pintext, content, "%s", "g");
                    content = pintext;
                }

                addTextElement(dir, content, fontstyle);
            }
        }
    }

    token = token->getNextToken();
    if (token == NULL) {
        return;
    }
    while (token && !token->isData()) {
        token = token->getNextToken();
    }
    if (token == NULL) {
        return;
    }
    if (!token->isNull()) {
        return;
    }
    // re-run this function on null tokens after the main note since
    // there may be dynamics unattached to a note (for various often
    // legitimate reasons).  Maybe make this more efficient later, such as
    // do a separate parse of dynamics data in a different loop.
    processDynamics(token, staffindex);
}

//////////////////////////////
//
// HumdrumInput::setAttachmentType -- attach at a timestamp if a regular note/rest;
//    otherwise attach with startid if a grace note.
//

template <class ELEMENT> void HumdrumInput::setAttachmentType(ELEMENT *element, hum::HTp token)
{
    hum::HumNum linedur = token->getLine()->getDuration();
    int staffindex = m_rkern[token->getTrack()];
    if (token->isNull()) {
        // Element cannot be attached to anything, so use timestamp;
        hum::HumNum barstamp = getMeasureTstamp(token, staffindex);
        element->SetTstamp(barstamp.getFloat());
    }
    else if (token->isBarline()) {
        hum::HumNum barstamp = getMeasureTstamp(token, staffindex);
        element->SetTstamp(barstamp.getFloat());
    }
    else if (linedur == 0) {
        // Element is attached to non-durational item (grace note, clef, time signature, etc).
        attachToToken(element, token);
    }
    else {
        int staffindex = m_rkern[token->getTrack()];
        hum::HumNum barstamp = getMeasureTstamp(token, staffindex);
        element->SetTstamp(barstamp.getFloat());
    }
}

//////////////////////////////
//
// HumdrumInput::attachToToken -- Used to attach items to grace notes
//    rather than using timestamps.  Null tokens are not allowed as input.
//

template <class ELEMENT> void HumdrumInput::attachToToken(ELEMENT *element, hum::HTp token)
{
    if (token->isNull()) {
        cerr << "ERROR: Cannot input null tokens into HumdrumInput::attachToToken() function." << endl;
        return;
    }
    if (token->isChord()) {
        element->SetStartid("#" + getLocationId("chord", token));
    }
    else if (token->isRest()) {
        element->SetStartid("#" + getLocationId("rest", token));
    }
    else if (token->isData()) {
        element->SetStartid("#" + getLocationId("note", token));
    }
    else if (token->isClef()) {
        element->SetStartid("#" + getLocationId("clef", token));
    }
}

//////////////////////////////
//
// HumdrumInput::getLeftNoteDuration --
//

hum::HumNum HumdrumInput::getLeftNoteDuration(hum::HTp token)
{
    hum::HumNum output = 0;
    hum::HTp current = token;
    while (current) {
        if (!current->isKernLike()) {
            current = current->getPreviousFieldToken();
            continue;
        }
        if (current->isNull()) {
            current = current->getPreviousFieldToken();
            continue;
        }
        output = hum::Convert::recipToDuration(current);
        break;
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::hasLayoutParameter -- True if there is a layout parameter
//   (regardless of whether or not it has a value).
//

bool HumdrumInput::hasLayoutParameter(hum::HTp token, const std::string &category, const std::string &param)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key != param) {
                continue;
            }
            if (value == "0") {
                return false;
            }
            if (value == "false") {
                return false;
            }
            return true;
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::hasAboveParameter -- true if has an "a" parameter or has a "Z" parameter set to anything.
//

bool HumdrumInput::hasAboveParameter(hum::HTp token, const std::string &category)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "a") {
                return true;
            }
            if (key == "Z") {
                return true;
            }
        }
    }
    return false;
}

// Output will not be changed if no explicit staff placement.

bool HumdrumInput::hasAboveParameter(hum::HTp token, const std::string &category, int &output)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "a") {
                if (value == "true") {
                    // below the attached staff
                    output = 0;
                }
                else if (!value.empty()) {
                    // b=2 means below the staff below the attached staff.
                    if (isdigit(value[0])) {
                        output = stoi(value);
                        if (output) {
                            output = -(output - 1);
                        }
                    }
                }
                return true;
            }
            if (key == "Y") {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::hasCenterParameter -- true if has a "c" parameter is present with optional staff adjustment.
//

bool HumdrumInput::hasCenterParameter(hum::HTp token, const std::string &category, int &output)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "c") {
                if (value == "true") {
                    // below the attached staff
                    output = 0;
                }
                else if (!value.empty()) {
                    // c=2 means below the staff below the attached staff.
                    if (isdigit(value[0])) {
                        output = stoi(value);
                        if (output) {
                            output = -(output - 1);
                        }
                    }
                }
                return true;
            }
            if (key == "Y") {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::getLayoutParameter -- Get an attached layout parameter
//   for a token.  Move this variant into HumdrumToken class at some point.
//
//     trueString = value to return if there is a parameter but the
//                  value is empty.
//     falseString = value to return if there is no parameter.
//                   default value = ""
//

string HumdrumInput::getLayoutParameter(hum::HTp token, const std::string &category, const std::string &catkey,
    const std::string &trueString, const std::string &falseString)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return falseString;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            if (key == catkey) {
                std::string value = hps->getParameterValue(q);
                if (value.empty()) {
                    return trueString;
                }
                else {
                    return value;
                }
            }
        }
    }
    return falseString;
}

//////////////////////////////
//
// HumdrumInput::getLoColor -- Get any layout color value.
//

std::string HumdrumInput::getLoColor(hum::HTp token, const std::string &category, int subtoken)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return "";
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "color") {
                return value;
            }
        }
    }
    return "";
}

//////////////////////////////
//
// HumdrumInput::hasBelowParameter -- true if has an "b" parameter or has a "Z" parameter set to anything.
//

bool HumdrumInput::hasBelowParameter(hum::HTp token, const std::string &category)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "b") {
                return true;
            }
            if (key == "Y") {
                return true;
            }
        }
    }
    return false;
}

// Output will not be changed if no explicit staff placement.

bool HumdrumInput::hasBelowParameter(hum::HTp token, const std::string &category, int &output)
{
    int lcount = token->getLinkedParameterSetCount();
    if (lcount == 0) {
        return 0;
    }

    for (int p = 0; p < token->getLinkedParameterSetCount(); ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            std::string value = hps->getParameterValue(q);
            if (key == "b") {
                if (value == "true") {
                    // below the attached staff
                    output = 0;
                }
                else if (!value.empty()) {
                    // b=2 means below the staff below the attached staff.
                    if (isdigit(value[0])) {
                        output = stoi(value);
                        if (output) {
                            output = -(output - 1);
                        }
                    }
                }
                return true;
            }
            if (key == "Y") {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::getMeasureDifference --
//

int HumdrumInput::getMeasureDifference(hum::HTp starttok, hum::HTp endtok)
{
    if (endtok == NULL) {
        return 0;
    }
    hum::HumdrumLine *line = starttok->getOwner();
    if (line == NULL) {
        return 0;
    }
    hum::HumdrumFile *file = line->getOwner();
    if (file == NULL) {
        return 0;
    }
    hum::HumdrumFile &infile = *file;
    int startline = starttok->getLineIndex();
    int endline = endtok->getLineIndex();
    int counter = 0;
    for (int i = startline; i <= endline; ++i) {
        if (infile[i].isBarline()) {
            counter++;
        }
    }
    return counter;
}

int HumdrumInput::getMeasureDifference(
    hum::HTp starttok, hum::HumNum meterunit, hum::HumNum tieduration, hum::HumNum &tstamp)
{
    hum::HumdrumLine *line = starttok->getOwner();
    if (line == NULL) {
        return 0;
    }
    hum::HumdrumFile *file = line->getOwner();
    if (line == NULL) {
        return 0;
    }
    hum::HumdrumFile &infile = *file;
    hum::HumNum endtime = starttok->getDurationFromStart() + tieduration;
    int startline = starttok->getLineIndex();
    int counter = 0;
    int i = startline;
    int lastBarline = -1;
    while ((i < infile.getLineCount()) && (infile[i].getDurationFromStart() < endtime)) {
        if (infile[i].isBarline()) {
            // probably deal with invisible barlines
            counter++;
            lastBarline = i;
        }
        i++;
    }

    if (lastBarline == -1) {
        hum::HumNum frombar = starttok->getDurationFromBarline();
        tstamp = frombar + tieduration;
        tstamp *= meterunit;
        tstamp /= 4;
        tstamp += 1;
        return 0;
    }

    // If the time signature bottom has changed, meterunit will need to be updated...
    hum::HumNum lastbartime = infile[lastBarline].getDurationFromStart();
    tstamp = endtime - lastbartime;
    tstamp *= meterunit;
    tstamp /= 4;
    tstamp += 1;

    return counter;
}

//////////////////////////////
//
// HumdrumInput::getCrescendoEnd --
//

hum::HTp HumdrumInput::getCrescendoEnd(hum::HTp token)
{
    return getHairpinEnd(token, "[");
}

//////////////////////////////
//
// HumdrumInput::getDecrescendoEnd --
//

hum::HTp HumdrumInput::getDecrescendoEnd(hum::HTp token)
{
    return getHairpinEnd(token, "]");
}

//////////////////////////////
//
// HumdrumInput::getHairpinEnd --
//

hum::HTp HumdrumInput::getHairpinEnd(hum::HTp token, const std::string &endchar)
{
    if (token == NULL) {
        return NULL;
    }
    token = token->getNextNonNullDataToken();
    int badtoken = 0;
    while (token != NULL) {
        if (token->find(endchar) != std::string::npos) {
            return token;
        }
        badtoken = 0;
        for (int i = 0; i < (int)token->size(); ++i) {
            if (isalpha((*token)[i])) {
                badtoken = 1;
            }
            else if ((*token)[i] == '<') {
                badtoken = 1;
            }
            else if ((*token)[i] == '>') {
                badtoken = 1;
            }
            if (badtoken) {
                // maybe return the bad token for a weak ending
                // to a hairpin...
                return NULL;
            }
        }
        token = token->getNextNonNullDataToken();
    }
    return NULL;
}

//////////////////////////////
//
// HumdrumInput::getMeasureTstamp --
//     default value: fract = 0.0;
//

hum::HumNum HumdrumInput::getMeasureTstamp(hum::HTp token, int staffindex, hum::HumNum fract)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum qbeat = token->getDurationFromBarline();
    if (fract > 0) {
        // what is this for? Causes problems with pedal markings.
        // qbeat += fract * token->getDuration().getAbs();
    }
    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
    // if (ss[staffindex].meter_bottom == 0) {
    //  mfactor = 1;
    //  mfactor /= 8;
    // }
    hum::HumNum mbeat = qbeat * mfactor + 1;
    return mbeat;
}

hum::HumNum HumdrumInput::getMeasureTstamp(hum::HTp token, hum::HumNum extraduration, int staffindex, hum::HumNum fract)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum qbeat = token->getDurationFromBarline();
    qbeat += extraduration;
    if (fract > 0) {
        // what is this for? Causes problems with pedal markings.
        // qbeat += fract * token->getDuration().getAbs();
    }
    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
    // if (ss[staffindex].meter_bottom == 0) {
    //  mfactor = 1;
    //  mfactor /= 8;
    // }
    hum::HumNum mbeat = qbeat * mfactor + 1;
    return mbeat;
}

//////////////////////////////
//
// HumdrumInput::getMeasureFactor -- Get the metric unit of the current measure.
//

hum::HumNum HumdrumInput::getMeasureFactor(int staffindex)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
    return mfactor;
}

//////////////////////////////
//
// HumdrumInput::getMeasureTstampPlusDur --  Similar to getMeasureTstamp, but also include
//     duration of token (to get endpoint of token in measure).
//     default value: fract = 0.0;
//

hum::HumNum HumdrumInput::getMeasureTstampPlusDur(hum::HTp token, int staffindex, hum::HumNum fract)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum qbeat = token->getDurationFromBarline() + token->getDuration();
    if (fract > 0) {
        // what is this for? Causes problems with pedal markings.
        // qbeat += fract * token->getDuration().getAbs();
    }
    hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
    // if (ss[staffindex].meter_bottom == 0) {
    //  mfactor = 1;
    //  mfactor /= 8;
    // }
    hum::HumNum mbeat = qbeat * mfactor + 1;
    return mbeat;
}

//////////////////////////////
//
// HumdrumInput::getMeasureEndTstamp -- Return the tstamp of the end of the
//     measure (the next barline). (@meter.count + 1)
//     default value: fract = 0.0;
//

hum::HumNum HumdrumInput::getMeasureEndTstamp(int staffindex)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    return ss[staffindex].meter_top + 1;
}

/////////////////////////////
//
// HumdrumInput::addSmuflSymbol -- Add a SMuFL symbol to some
//      text-based element (such as <rend>).  Humdrum music symbol names assumed
//      as input, such as "sc" for segnum contruentiae..

template <class ELEMENT> void HumdrumInput::addMusicSymbol(ELEMENT *element, const std::string &musictext)
{
    std::vector<std::string> smufltext = convertMusicSymbolNameToSmuflName(musictext);
    if (smufltext.empty()) {
        // nothing to do: treat as plain text and add in calling function.
        return;
    }

    int counter = 0;
    for (int i = 0; i < (int)smufltext.size(); i++) {
        if (smufltext.at(i).empty()) {
            continue;
        }
        std::string name = smufltext.at(i);
        if (counter) {
            // Add a space element between music symbols.
            if (smufltext.at(i) == "metAugmentationDot") {
                addTextElement(element, m_textAugmentationDotSpacer);
            }
            else {
                addTextElement(element, m_textSmuflSpacer);
            }
        }
        ++counter;

        Symbol *symbol = new Symbol();
        setSmuflContent(symbol, name);
        setFontsize(symbol, name, musictext);
        element->AddChild(symbol);
    }
}

/////////////////////////////
//
// HumdumInput::addTextElement -- Append text to a regular element.
//   default value: fontstyle == ""
//

template <class ELEMENT>
void HumdrumInput::addTextElement(
    ELEMENT *element, const std::string &content, const std::string &fontstyle, bool addSpacer)
{
    Text *text = new Text();
    std::string myfontstyle = fontstyle;

    std::string data = content;

    if (data.find("<i>") != std::string::npos) {
        // Convert <i>..</i> into italic.  Currently only entire syllable
        // can be italic (no partially italics).
        myfontstyle = "italic";
        hum::HumRegex hre;
        hre.replaceDestructive(data, "", "<i>", "g");
        hre.replaceDestructive(data, "", "</i>", "g");
    }

    if (element->GetClassName() == "Syl") {
        // Approximate centering of single-letter text on noteheads.
        // currently the text is left justified to the left edge of the notehead.
        if ((data.size() == 1) && addSpacer) {
            data = "&#160;" + data;
        }
    }

    // Parse [ASCII] music codes to route to VerovioText font rends:
    hum::HumRegex hre;
    if (hre.search(data, "^(.*?)(\\[.*?\\])(.*)$")) {
        std::string pretext = hre.getMatch(1);
        std::string rawmusictext = hre.getMatch(2);
        std::vector<std::string> musictext = convertMusicSymbolNameToSmuflName(rawmusictext);
        std::string posttext = hre.getMatch(3);
        if (pretext == "\\n") {
            Lb *lb = new Lb();
            element->AddChild(lb);
            pretext = "";
        }
        if (musictext.empty()) {
            hum::HumRegex hre2;
            std::string newtext = rawmusictext;
            hre.replaceDestructive(newtext, "&#91;", "\\[", "g");
            hre.replaceDestructive(newtext, "&#93;", "\\]", "g");
            pretext += newtext;
        }

        if (!pretext.empty()) {
            pretext = unescapeHtmlEntities(pretext);
            hre.replaceDestructive(pretext, "[", "&#91;", "g");
            hre.replaceDestructive(pretext, "]", "&#93;", "g");
            Rend *rend = new Rend();
            element->AddChild(rend);

            rend->AddChild(text);
            text->SetText(UTF8to32(pretext));
            setFontStyle(rend, myfontstyle);
            // addTextElement(element, pretext, myfontstyle, addSpacer);
        }
        if (!musictext.empty()) {
            addMusicSymbol(element, rawmusictext);
        }
        if (!posttext.empty()) {
            addTextElement(element, posttext, myfontstyle, addSpacer);
            return;
        }
        else {
            return;
        }
    }

    data = escapeFreeAmpersand(data);
    data = unescapeHtmlEntities(data);

    std::vector<std::string> pieces;
    hre.split(pieces, data, "\\\\n");

    for (int i = 0; i < (int)pieces.size(); ++i) {
        data = pieces[i];
        text->SetText(UTF8to32(data));

        if (myfontstyle.empty()) {
            if (text != NULL) {
                element->AddChild(text);
            }
        }
        else {
            if (text != NULL) {
                Rend *rend = new Rend();
                element->AddChild(rend);
                rend->AddChild(text);
                setFontStyle(rend, myfontstyle);
            }
        }

        if (i < (int)pieces.size() - 1) {
            // Need to add another text element, but add lb before it.
            Lb *lb = new Lb();
            element->AddChild(lb);
            text = new Text();
        }
    }
}

//////////////////////////////
//
// HumdrumInput::setFontStyle --
//

void HumdrumInput::setFontStyle(Rend *rend, const std::string &fontstyle)
{
    if (fontstyle == "normal") {
        rend->SetFontstyle(FONTSTYLE_normal);
    }
    else if (fontstyle == "bold") {
        rend->SetFontweight(FONTWEIGHT_bold);
        rend->SetFontstyle(FONTSTYLE_normal);
    }
    else if (fontstyle == "bold-italic") {
        rend->SetFontweight(FONTWEIGHT_bold);
    }
    else if (fontstyle == "italic") {
        rend->SetFontstyle(FONTSTYLE_italic);
    }
}

//////////////////////////////
//
// escapeFreeAmpersand -- Convert & into &amp;, but do not mess
//    with other HTML/XML entity encodings such as &auml; or &#5432;.
//

std::string HumdrumInput::escapeFreeAmpersand(const std::string &value)
{
    std::string output;
    for (int i = 0; i < (int)value.size(); ++i) {
        if (value[i] != '&') {
            output += value[i];
            continue;
        }
        bool solo = false;
        int lastj = i;
        for (int j = i + 1; j < (int)value.size(); j++) {
            if (value[j] == ' ') {
                solo = true;
                break;
            }
            if (value[j] == '&') {
                solo = true;
                break;
            }
            if (value[j] == ';') {
                solo = false;
                break;
            }
            lastj = j;
        }
        if (lastj == (int)value.size() - 1) {
            solo = true;
        }
        if (solo == true) {
            output += "&amp;";
        }
        else {
            output += '&';
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::generateSlurId --
//

std::string HumdrumInput::generateSlurId(hum::HTp token, int count, int number)
{
    std::string id;
    if (token->isChord()) {
        id = "chord-L";
    }
    else {
        id = "note-L";
    }
    id += to_string(token->getLineNumber());
    id += "F";
    id += to_string(token->getFieldNumber());
    token->setValue("MEI", "xml:id", id);
    if (count > 1) {
        id += "N";
        id += to_string(number);
    }
    return id;
}

/////////////////////////////
//
// HumdrumInput::processSlurs --
//

void HumdrumInput::processSlurs(hum::HTp slurend)
{
    hum::HumRegex hre;

    int slurendcount = slurend->getValueInt("auto", "slurEndCount");
    if (slurendcount <= 0) {
        return;
    }

    // slurstarts: indexed by slur end number (NB: 0 position not used).
    // pair contains the slur start enumeration and the start token.
    std::vector<pair<int, hum::HTp>> slurstartlist;
    slurstartlist.resize(slurendcount + 1);
    for (int i = 1; i <= slurendcount; ++i) {
        slurstartlist[i].first = slurend->getSlurStartNumber(i);
        slurstartlist[i].second = slurend->getSlurStartToken(i);
    }

    for (int i = 1; i <= slurendcount; ++i) {
        hum::HTp slurstart = slurstartlist[i].second;
        if (!slurstart) {
            continue;
        }
        int slurstartnumber = slurstartlist[i].first;
        int slurendnumber = i;
        int slurstartcount = slurstart->getValueInt("auto", "slurStartCount");

        int mindex;
        std::string mindexstring = slurstart->getValue("MEI", "measureIndex");
        if (mindexstring == "") {
            // cross-layer sluring into later layer.  The beginning of the slur
            // is in the same measure since it has not yet been processed.
            mindex = slurend->getValueInt("MEI", "measureIndex");
        }
        else {
            mindex = slurstart->getValueInt("MEI", "measureIndex");
        }

        // Check if there is a "y" marker immediately after the slur start, or
        // the notes at both ends of the slur are invisible, unless there is an
        // "X" immediately after the slur start.
        bool isInvisible = checkIfSlurIsInvisible(slurstart, slurstartnumber, slurend, slurendnumber);
        if (isInvisible) {
            continue;
        }

        Measure *startmeasure = m_measures[mindex];
        Slur *slur = new Slur();
        addSlurLineStyle(slur, slurstart, slurstartnumber);

        std::string startid = slurstart->getValue("MEI", "xml:id");
        std::string endid = slurend->getValue("MEI", "xml:id");

        // start ID can sometimes not be set yet due to cross layer slurs.
        if (startid.empty()) {
            startid = generateSlurId(slurstart, slurstartcount, slurstartnumber);
        }
        if (endid.empty()) {
            endid = generateSlurId(slurend, slurendcount, slurendnumber);
        }

        slur->SetStartid("#" + startid);
        slur->SetEndid("#" + endid);
        setSlurLocationId(slur, slurstart, slurend, slurstartnumber);

        startmeasure->AddChild(slur);
        if (slurstart->getTrack() == slurend->getTrack()) {
            // If the slur starts and ends on different staves,
            // do not specify the staff attribute, but later
            // add a list of the two staves involved.
            int staff = m_currentstaff;
            if (m_signifiers.above) {
                std::string sabove = "[a-g]+[-n#]*[xy]*";
                sabove += m_signifiers.above;
                if (hre.search(slurstart, sabove)) {
                    staff--;
                    if (staff < 1) {
                        staff = 1;
                    }
                }
            }
            if (m_signifiers.below) {
                std::string sbelow = "[a-g]+[-n#]*[xy]*";
                sbelow += m_signifiers.below;
                if (hre.search(slurstart, sbelow)) {
                    staff++;
                }
            }
            setStaff(slur, staff);
        }

        setLayoutSlurDirection(slur, slurstart);

        if (slurendnumber < 0) {
            continue;
        }

        // Calculate if the slur should be forced above or below
        // this is the case for doubly slured chords.  Only the first
        // two slurs between a pair of notes/chords will be oriented
        // (other slurs will need to be manually adjusted and probably
        // linked to individual notes to avoid overstriking the first
        // two slurs.

        if (slurendcount > 1) {
            int found = -1;
            for (int j = 1; j <= slurendcount; j++) {
                if (i == j) {
                    continue;
                }
                if (slurstartlist[i].second == slurstartlist[j].second) {
                    found = j;
                    break;
                }
            }
            if (found > 0) {
                if (found < i) {
                    slur->SetCurvedir(curvature_CURVEDIR_above);
                }
                else {
                    slur->SetCurvedir(curvature_CURVEDIR_below);
                }
            }
        }

        if (m_signifiers.above) {
            int count = 0;
            for (int k = 0; k < (int)slurstart->size() - 1; k++) {
                if (slurstart->at(k) == '(') {
                    count++;
                }
                if (count == slurstartnumber) {
                    if (slurstart->at(k + 1) == m_signifiers.above) {
                        slur->SetCurvedir(curvature_CURVEDIR_above);
                        appendTypeTag(slur, "placed");
                    }
                    break;
                }
            }
        }
        if (m_signifiers.below) {
            int count = 0;
            for (int k = 0; k < (int)slurstart->size() - 1; k++) {
                if (slurstart->at(k) == '(') {
                    count++;
                }
                if (count == slurstartnumber) {
                    if (slurstart->at(k + 1) == m_signifiers.below) {
                        slur->SetCurvedir(curvature_CURVEDIR_below);
                        appendTypeTag(slur, "placed");
                    }
                    break;
                }
            }
        }
    }
}

//////////////////////////////
//
// setLayoutSlurDirection --
//

void HumdrumInput::setLayoutSlurDirection(Slur *slur, hum::HTp token)
{
    if (hasAboveParameter(token, "S")) {
        slur->SetCurvedir(curvature_CURVEDIR_above);
        appendTypeTag(slur, "placed");
    }
    else if (hasBelowParameter(token, "S")) {
        slur->SetCurvedir(curvature_CURVEDIR_below);
        appendTypeTag(slur, "placed");
    }
}

/////////////////////////////
//
// HumdrumInput::checkIfSlurIsInvisible --
//

bool HumdrumInput::checkIfSlurIsInvisible(hum::HTp stoken, int snumber, hum::HTp etoken, int enumber)
{
    int tsize = (int)stoken->size();
    int counter = 0;
    bool hasy = false;
    bool hasX = false;

    for (int i = 0; i < tsize - 1; ++i) {
        if (stoken->at(i) == '(') {
            counter++;
        }
        else {
            continue;
        }
        if (counter == snumber) {
            if (stoken->at(i + 1) == 'y') {
                hasy = true;
            }
            else if (stoken->at(i + 1) == 'X') {
                hasX = true;
            }
        }
    }

    if (hasy) {
        return true;
    }
    if (hasX) {
        return false;
    }

    if ((stoken->find("yy") != std::string::npos) && (etoken->find("yy") != std::string::npos)) {
        bool schord = stoken->isChord();
        bool echord = etoken->isChord();
        if (!(schord || echord)) {
            return true;
        }

        int scount = stoken->getSubtokenCount();
        int ecount = stoken->getSubtokenCount();

        int syycount = 0;
        int eyycount = 0;

        if (scount == 1) {
            if (stoken->find("yy") != std::string::npos) {
                syycount = 1;
            }
            else {
                for (int i = 1; i < (int)stoken->size(); ++i) {
                    if ((stoken->at(i) == 'y') && (stoken->at(i - 1) == 'y')) {
                        syycount++;
                        i++;
                    }
                }
            }
        }

        if (ecount == 1) {
            if (etoken->find("yy") != std::string::npos) {
                eyycount = 1;
            }
            else {
                for (int i = 1; i < (int)etoken->size(); ++i) {
                    if ((etoken->at(i) == 'y') && (etoken->at(i - 1) == 'y')) {
                        eyycount++;
                        i++;
                    }
                }
            }
        }

        if ((scount == syycount) && (ecount == eyycount)) {
            // all notes in both chords are invisible so do not
            // show the slur connecting them.
            return true;
        }
    }

    return false;
}

/////////////////////////////
//
// HumdrumInput::processPhrases --
//

void HumdrumInput::processPhrases(hum::HTp phraseend)
{
    int startcount = phraseend->getValueInt("auto", "phraseStartCount");
    if (startcount <= 0) {
        return;
    }

    // phrasestarts contains a list of the correctly paired phrases
    // attached to the note/chord.  Deal with unopened phrases
    // here later (such as an excerpt of music where the opening
    // of the phrase is not present in the data).
    std::vector<hum::HTp> phrasestarts;
    for (int i = 0; i < startcount; ++i) {
        hum::HTp tok;
        tok = phraseend->getPhraseStartToken(i + 1);
        if (tok) {
            phrasestarts.push_back(tok);
        }
    }

    // phraseindex contains a list of the indexes into phrasestarts,
    // with all identical phrase starts placed on the first
    // position that the note/chord is found in the phrasestarts list.
    std::vector<std::vector<int>> phraseindex;
    phraseindex.resize(phrasestarts.size());
    for (int i = 0; i < (int)phrasestarts.size(); ++i) {
        for (int j = 0; j <= i; j++) {
            if (phrasestarts[i] == phrasestarts[j]) {
                phraseindex[j].push_back(i);
                break;
            }
        }
    }

    std::vector<bool> indexused(32, false);

    std::vector<pair<int, bool>> phraseendnoteinfo;
    extractPhraseNoteAttachmentInformation(phraseendnoteinfo, phraseend, '}');

    int endsubtokcount = phraseend->getSubtokenCount();
    std::vector<int> endpitches;
    for (int i = 0; i < endsubtokcount; ++i) {
        std::string subtok = phraseend->getSubtoken(i);
        if (subtok.find("r") != std::string::npos) {
            endpitches.push_back(0);
        }
        else {
            endpitches.push_back(hum::Convert::kernToBase7(subtok));
        }
    }
    std::vector<pair<int, int>> endchordsorted;
    endchordsorted.reserve(endsubtokcount);
    pair<int, int> v;
    for (int i = 0; i < endsubtokcount; ++i) {
        v.first = endpitches[i];
        v.second = i;
        endchordsorted.push_back(v);
    }
    std::sort(endchordsorted.begin(), endchordsorted.end());

    int startsubtokcount;
    std::vector<int> startpitches;

    for (int i = 0; i < (int)phraseindex.size(); ++i) {
        std::vector<pair<int, bool>> phrasestartnoteinfo;
        extractPhraseNoteAttachmentInformation(phrasestartnoteinfo, phrasestarts.at(i), '{');

        startsubtokcount = phrasestarts[i]->getSubtokenCount();
        startpitches.clear();
        for (int j = 0; j < startsubtokcount; j++) {
            std::string subtok = phrasestarts[i]->getSubtoken(j);
            if (subtok.find("r") != std::string::npos) {
                startpitches.push_back(0);
            }
            else {
                startpitches.push_back(hum::Convert::kernToBase7(subtok));
            }
        }
        std::vector<std::pair<int, int>> startchordsorted;
        startchordsorted.reserve(startsubtokcount);

        pair<int, int> v;
        for (int i = 0; i < startsubtokcount; ++i) {
            v.first = startpitches[i];
            v.second = i;
            startchordsorted.push_back(v);
        }
        std::sort(startchordsorted.begin(), startchordsorted.end());
        for (int j = 0; j < (int)phraseindex[i].size(); j++) {
            int ndex = -1;
            if (phraseindex.size() > 1) {
                ndex = j;
            }
            hum::HTp phrasestart = phrasestarts[phraseindex[i][j]];

            std::vector<pair<int, bool>> phrasestartnoteinfo;
            extractPhraseNoteAttachmentInformation(phrasestartnoteinfo, phrasestart, '{');
            if (!phrasestart) {
                // should never occur...
                return;
            }

            int mindex;
            std::string mindexstring = phrasestart->getValue("MEI", "measureIndex");
            if (mindexstring == "") {
                // cross-layer phraseing into later layer.  The beginning of the phrase
                // is in the same measure since it has not yet been processed.
                mindex = phraseend->getValueInt("MEI", "measureIndex");
            }
            else {
                mindex = phrasestart->getValueInt("MEI", "measureIndex");
            }

            Measure *startmeasure = m_measures[mindex];

            if (phraseIsInvisible(phrasestart, ndex)) {
                continue;
            }

            std::string isslur = m_signifiers.phrase_slur;
            if (isslur.empty()) {
                isslur = phrasestart->getLayoutParameter("P", "slur", ndex);
            }
            if (!isslur.empty()) {
                // insert phrase as slur
                Slur *slur = new Slur();
                insertPhrase(slur, phrasestart, phraseend, startmeasure, startchordsorted, endchordsorted,
                    phrasestartnoteinfo, phraseendnoteinfo, ndex, phraseindex, i, j, startpitches, endpitches,
                    indexused);
                // Force phrase slur above the staff:
                slur->SetCurvedir(curvature_CURVEDIR_above);
            }
            else {
                // insert phrase as bracket
                BracketSpan *bracket = new BracketSpan();
                insertPhrase(bracket, phrasestart, phraseend, startmeasure, startchordsorted, endchordsorted,
                    phrasestartnoteinfo, phraseendnoteinfo, ndex, phraseindex, i, j, startpitches, endpitches,
                    indexused);
                // bracket will not be drawn without the following line:
                bracket->SetFunc("phrase");
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::phraseIsInvisible -- Returns true if the phrase mark
//    should not be rendered.
//

bool HumdrumInput::phraseIsInvisible(hum::HTp token, int pindex)
{
    std::string none = token->getLayoutParameter("P", "none", pindex);
    if (!none.empty()) {
        return true;
    }

    std::string style = token->getLayoutParameter("P", "brack", pindex);
    if (style.empty()) {
        style = token->getLayoutParameter("P", "paren", pindex);
    }
    if (style.empty()) {
        style = token->getLayoutParameter("P", "dot", pindex);
    }
    if (style.empty()) {
        style = token->getLayoutParameter("P", "dash", pindex);
    }
    if (style.empty()) {
        style = token->getLayoutParameter("P", "slur", pindex);
    }

    if (style.empty()) {
        if (m_signifiers.phrase_style == "none") {
            return true;
        }
    }
    if (pindex < 0) {
        pindex = 0;
    }

    int counter = -1;
    for (int i = 0; i < (int)token->size() - 1; ++i) {
        char ch = token->at(i);
        if (ch != '{') {
            continue;
        }
        counter++;
        if (counter != pindex) {
            continue;
        }
        if (token->at(i + 1) == 'y') {
            return true;
        }
        break;
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::insertPhrase -- insert a phrase (which can be either
//    rendered as a slur or as a bracketSpan (or not inserted if the
//    style is "none").
//

template <class ELEMENT>
void HumdrumInput::insertPhrase(ELEMENT phrase, hum::HTp phrasestart, hum::HTp phraseend, Measure *startmeasure,
    std::vector<pair<int, int>> &endchordsorted, std::vector<std::pair<int, int>> &startchordsorted,
    std::vector<pair<int, bool>> &phrasestartnoteinfo, std::vector<pair<int, bool>> &phraseendnoteinfo, int ndex,
    std::vector<std::vector<int>> &phraseindex, int i, int j, std::vector<int> &startpitches,
    std::vector<int> &endpitches, std::vector<bool> &indexused)
{

    appendTypeTag(phrase, "phrase");

    std::string style = m_signifiers.phrase_style;

    std::string teststyle = phrasestart->getLayoutParameter("P", "brack", ndex);
    if (!teststyle.empty()) {
        style = "brack";
    }
    else {
        teststyle = phrasestart->getLayoutParameter("P", "dot", ndex);
        if (!teststyle.empty()) {
            style = "dot";
        }
        else {
            teststyle = phrasestart->getLayoutParameter("P", "dash", ndex);
            if (!teststyle.empty()) {
                style = "dash";
            }
            else {
                teststyle = phrasestart->getLayoutParameter("P", "open", ndex);
                if (!teststyle.empty()) {
                    style = "open";
                }
                else {
                    teststyle = phrasestart->getLayoutParameter("P", "wavy", ndex);
                    if (!teststyle.empty()) {
                        style = "wavy";
                    }
                }
            }
        }
    }

    if (style == "brack") {
        phrase->SetLform(LINEFORM_solid);
    }
    else if (style == "dot") {
        phrase->SetLform(LINEFORM_dotted);
    }
    else if (style == "dash") {
        phrase->SetLform(LINEFORM_dashed);
    }
    else if (style == "wavy") {
        // not yet implemented in verovio (will display as open bracket)
        phrase->SetLform(LINEFORM_wavy);
    }

    std::string color = m_signifiers.phrase_color;
    std::string testcolor = phrasestart->getLayoutParameter("P", "color", ndex);
    if (!testcolor.empty()) {
        color = testcolor;
    }
    if (!color.empty()) {
        phrase->SetColor(color);
    }

    // start ID can sometimes not be set yet due to cross layer phrases.
    std::string startid = phrasestart->getValue("MEI", "xml:id");
    std::string endid = phraseend->getValue("MEI", "xml:id");

    if (startid == "") {
        if (phrasestart->isChord()) {
            startid = "chord-L";
        }
        else {
            startid = "note-L";
        }
        startid += to_string(phrasestart->getLineNumber());
        startid += "F";
        startid += to_string(phrasestart->getFieldNumber());
        phrasestart->setValue("MEI", "xml:id", startid);
        startid = phrasestart->getValue("MEI", "xml:id");
    }

    if (phraseindex[i].size() > 1) {
        if (endpitches.size() > 1) {
            calculateNoteIdForSlur(endid, endchordsorted, j);
        }
        if (startpitches.size() > 1) {
            calculateNoteIdForSlur(startid, startchordsorted, j);
        }
    }

    if (phraseendnoteinfo.at(i).second) {
        if (endid.find("chord") != std::string::npos) {
            hum::HumRegex hre;
            hre.replaceDestructive(endid, "note", "chord");
            endid += "S";
            endid += to_string(phraseendnoteinfo[i].first + 1);
        }
    }

    if (phrasestartnoteinfo.at(j).second) {
        if (startid.find("chord") != std::string::npos) {
            hum::HumRegex hre;
            hre.replaceDestructive(startid, "note", "chord");
            startid += "S";
            startid += to_string(phrasestartnoteinfo[i].first + 1);
        }
    }

    phrase->SetEndid("#" + endid);
    phrase->SetStartid("#" + startid);

    int slurstartnumber = 1; // hardwired to 1 for now.
    setSlurLocationId(phrase, phrasestart, phraseend, slurstartnumber, "phrase");

    startmeasure->AddChild(phrase);
    if (phrasestart->getTrack() == phraseend->getTrack()) {
        // If the phrase starts and ends on different staves,
        // do not specify the staff attribute, but later
        // add a list of the two staves involved.
        setStaff(phrase, m_currentstaff);
    }

    // if (hasAboveParameter(phrasestart, "S")) {
    //     slur->SetCurvedir(curvature_CURVEDIR_above);
    // }
    // else if (hasBelowParameter(phrasestart, "S")) {
    //     slur->SetCurvedir(curvature_CURVEDIR_below);
    // }

    std::string eid = phraseend->getValue("auto", "id");
    // int phraseidx = getSlurEndIndex(phrasestart, eid, indexused);
    int phraseidx = 0;
    if (phraseidx < 0) {
        return;
    }
    indexused.at(phraseidx) = true;

    // Calculate if the phrase should be forced above or below
    // this is the case for doubly phrased chords.  Only the first
    // two phrases between a pair of notes/chords will be oriented
    // (other phrases will need to be manually adjusted and probably
    // linked to individual notes to avoid overstriking the first
    // two phrases.
    // if (phraseindex[i].size() >= 2) {
    //     if (phrasestarts[phraseindex[i][0]] == phrasestarts[phraseindex[i][1]]) {
    //         if (j == 0) {
    //             phrase->SetCurvedir(curvature_CURVEDIR_above);
    //         }
    //         else {
    //             phrase->SetCurvedir(curvature_CURVEDIR_below);
    //         }
    //     }
    // }
}

//////////////////////////////
//
// HumdrumInput::addSlurLineStyle -- Add dotted or dashed line information to a
//    slur from layout parameters.
//

void HumdrumInput::addSlurLineStyle(Slur *element, hum::HTp token, int slurnumber)
{
    if (slurnumber < 2) {
        slurnumber = 1;
    }
    int slurindex = slurnumber - 1;
    std::string dashed = token->getLayoutParameter("S", "dash", slurindex);
    std::string dotted = token->getLayoutParameter("S", "dot", slurindex);
    if (!dotted.empty()) {
        element->SetLform(LINEFORM_dotted);
    }
    else if (!dashed.empty()) {
        element->SetLform(LINEFORM_dashed);
    }

    std::string color = token->getLayoutParameter("S", "color", slurindex);
    if (!color.empty()) {
        element->SetColor(color);
    }
}

//////////////////////////////
//
// HumdrumInput::addTieLineStyle -- Add dotted or dashed line information to a
//    tie from layout parameters.
//        Default parameter: index = 0.
//

void HumdrumInput::addTieLineStyle(Tie *tie, hum::HTp token, int noteindex)
{
    std::string tstring;
    if (noteindex < 0) {
        tstring = *token;
    }
    else {
        tstring = token->getSubtoken(noteindex);
    }

    std::string dashed = token->getLayoutParameter("T", "dash", noteindex);
    std::string dotted = token->getLayoutParameter("T", "dot", noteindex);
    if (!dotted.empty()) {
        tie->SetLform(LINEFORM_dotted);
    }
    else if (!dashed.empty()) {
        tie->SetLform(LINEFORM_dashed);
    }

    std::string color = token->getLayoutParameter("T", "color", noteindex);
    if (!color.empty()) {
        tie->SetColor(color);
    }
    std::string above = token->getLayoutParameter("T", "a", noteindex);
    if (!above.empty()) {
        tie->SetCurvedir(curvature_CURVEDIR_above);
    }
    else {
        std::string below = token->getLayoutParameter("T", "b", noteindex);
        if (!below.empty()) {
            tie->SetCurvedir(curvature_CURVEDIR_below);
        }
    }

    std::string marker1 = "[";
    std::string marker2 = "[";
    std::string marker3 = "_";
    std::string marker4 = "_";

    if (m_signifiers.above) {
        marker1 += m_signifiers.above;
        marker3 += m_signifiers.above;
    }
    if (m_signifiers.below) {
        marker2 += m_signifiers.below;
        marker4 += m_signifiers.below;
    }

    if (m_signifiers.above && tstring.find(marker1) != std::string::npos) {
        tie->SetCurvedir(curvature_CURVEDIR_above);
    }
    else if (m_signifiers.below && tstring.find(marker2) != std::string::npos) {
        tie->SetCurvedir(curvature_CURVEDIR_below);
    }
    else if (m_signifiers.above && tstring.find(marker3) != std::string::npos) {
        tie->SetCurvedir(curvature_CURVEDIR_above);
    }
    else if (m_signifiers.below && tstring.find(marker4) != std::string::npos) {
        tie->SetCurvedir(curvature_CURVEDIR_below);
    }
}

//////////////////////////////
//
// HumdrumInput::extractSlurNoteAttachmentInformation --
//   Vector indexed by slur (open or close depending on slurtype) with data being the subtoken number and
//   boolean for attached to note instead of chord.
//

void HumdrumInput::extractSlurNoteAttachmentInformation(
    std::vector<std::pair<int, bool>> &data, hum::HTp token, char slurtype)
{
    // slurtype == '(' for slur start
    // slurtype == ')' for slur end
    data.clear();
    int subtokindex = 0;
    int slurnumber = 0;
    int toksize = (int)token->size();
    bool notestate;
    for (int i = 0; i < toksize; ++i) {
        if (token->at(i) == ' ') {
            subtokindex++;
        }
        else if (token->at(i) == ')') {
            slurnumber++;
            if (slurtype == ')') {
                notestate = getNoteStateSlur(token, slurnumber);
                data.emplace_back(std::make_pair(subtokindex, notestate));
            }
        }
        else if (token->at(i) == '(') {
            slurnumber++;
            if (slurtype == '(') {
                notestate = getNoteStateSlur(token, slurnumber);
                data.emplace_back(std::make_pair(subtokindex, notestate));
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::extractPhraseNoteAttachmentInformation --
//   Vector indexed by phrase (open or close depending on phrasetype) with data being the subtoken number and
//   boolean for attached to note instead of chord.
//

void HumdrumInput::extractPhraseNoteAttachmentInformation(
    std::vector<std::pair<int, bool>> &data, hum::HTp token, char phrasetype)
{
    // phrasetype == '{' for phrase start
    // phrasetype == '}' for phrase end
    data.clear();
    int subtokindex = 0;
    int phrasenumber = 0;
    int toksize = (int)token->size();
    bool notestate;
    for (int i = 0; i < toksize; ++i) {
        if (token->at(i) == ' ') {
            subtokindex++;
        }
        else if (token->at(i) == '}') {
            phrasenumber++;
            if (phrasetype == '}') {
                notestate = getNoteStatePhrase(token, phrasenumber);
                data.emplace_back(std::make_pair(subtokindex, notestate));
            }
        }
        else if (token->at(i) == '{') {
            phrasenumber++;
            if (phrasetype == '{') {
                notestate = getNoteStatePhrase(token, phrasenumber);
                data.emplace_back(std::make_pair(subtokindex, notestate));
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getNoteStateSlur -- Return any slur attachment to a note parameter in a layout command for slurs.
//

bool HumdrumInput::getNoteStateSlur(hum::HTp token, int slurnumber)
{
    std::string data = token->getSlurLayoutParameter("note", slurnumber - 1);
    if (data == "true") {
        return true;
    }
    else {
        return false;
    }
}

//////////////////////////////
//
// HumdrumInput::getNoteStatePhrase -- Return any phrase attachment to a note parameter in a layout command for phrases.
//

bool HumdrumInput::getNoteStatePhrase(hum::HTp token, int phrasenumber)
{
    std::string data = token->getPhraseLayoutParameter("note", phrasenumber - 1);
    if (data == "true") {
        return true;
    }
    else {
        return false;
    }
}

//////////////////////////////
//
// HumdrumInput::calculateNoteIdForSlur --
//

void HumdrumInput::calculateNoteIdForSlur(std::string &idstring, std::vector<pair<int, int>> &sortednotes, int index)
{
    int notecount = (int)sortednotes.size();
    hum::HumRegex hre;
    if (notecount == 1) {
        hre.replaceDestructive(idstring, "note-", "chord-");
    }

    /* Not attaching multiple slurs to note anymore, but leaving code
            here in case there is a later need for it:

        int position = 0;

        // odd:
        // input (index):    0 1 2 3 4
        // output       :    0 2 4 3 1
        // even:
        // input (index):    0 1 2 3
        // output       :    0 2 3 1

        if (index < (notecount + 1) / 2) {
            position = index * 2;
        }
        else {
            position = (notecount - index) * 2 - 1;
        }
        if (position < 0) {
            position += 100 * notecount;
            position %= notecount;
        }
        int subtoken = sortednotes.at(position).second;
        // collapse cases where there are more slurs than notes:
        subtoken = subtoken % notecount;
        // offset from 1 instead of 0:
        subtoken++;
        idstring += "S" + to_string(subtoken);
    */
}

/////////////////////////////
//
// HumdrumInput::getSlurEndNumber --
//

int HumdrumInput::getSlurEndNumber(hum::HTp startslur, int slurstartnumber)
{
    if (slurstartnumber < 1) {
        slurstartnumber = 1;
    }
    int startcount = startslur->getValueInt("auto", "slurStartCount");
    std::string parameter = "slurEndNumber";
    if (startcount > 1) {
        parameter += to_string(slurstartnumber);
    }

    int slurendnumber = startslur->getValueInt("auto", parameter);
    return slurendnumber;
}

/////////////////////////////
//
// HumdrumInput::insertMeterSigElement -- A time signature or meter signature
//    that starts after the beginning of a movement.
//

void HumdrumInput::insertMeterSigElement(
    std::vector<std::string> &elements, std::vector<void *> &pointers, std::vector<hum::HTp> &layerdata, int index)
{
    hum::HTp tsig = layerdata[index];
    if (!tsig) {
        return;
    }
    if (tsig->getDurationFromStart() <= 0) {
        return;
    }
    std::smatch matches;
    int count = -1;
    int unit = -1;
    if (regex_search(*tsig, matches, regex("^\\*M(\\d+)/(\\d+)"))) {
        count = stoi(matches[1]);
        unit = stoi(matches[2]);
    }
    else if (regex_search(*tsig, matches, regex("^\\*M(\\d+)"))) {
        count = stoi(matches[1]);
    }
    // deal with non-rational units here.
    if (count < 0) {
        return;
    }
    MeterSig *msig = new MeterSig();
    if (tsig) {
        setLocationId(msig, tsig);
    }
    appendElement(elements, pointers, msig);
    msig->SetCount({ { count }, MeterCountSign::None });
    if (unit > 0) {
        msig->SetUnit(unit);
    }
    // check for mensuration here.
}

//////////////////////////////
//
// HumdrumInput::getMeterSig -- Return the MeterSig element of
//   the given object, or if there is none create it and
//   return a pointer to it. ELEMENT can be ScoreDef or StaffDef.
//

template <class ELEMENT> MeterSig *HumdrumInput::getMeterSig(ELEMENT element)
{
    MeterSig *output = (MeterSig *)element->FindDescendantByType(ClassId::METERSIG);
    if (!output) {
        output = new MeterSig();
        element->AddChild(output);
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::getKeySig -- Return the KeySig element of
//   the given object, or if there is none create it and
//   return a pointer to it. ELEMENT can be ScoreDef or StaffDef.
//

template <class ELEMENT> KeySig *HumdrumInput::getKeySig(ELEMENT element)
{
    KeySig *output = (KeySig *)element->FindDescendantByType(ClassId::KEYSIG);
    if (!output) {
        output = new KeySig();
        element->AddChild(output);
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::getClef -- Return the Clef element of
//   the given object, or if there is none create it and
//   return a pointer to it. ELEMENT can be ScoreDef or StaffDef.
//

template <class ELEMENT> Clef *HumdrumInput::getClef(ELEMENT element)
{
    Clef *output = (Clef *)element->FindDescendantByType(ClassId::KEYSIG);
    if (!output) {
        output = new Clef();
        element->AddChild(output);
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::getMensur -- Return the Mensur element of
//   the given object, or if there is none create it and
//   return a pointer to it. ELEMENT can be ScoreDef or StaffDef.
//

template <class ELEMENT> Mensur *HumdrumInput::getMensur(ELEMENT element, hum::HTp token)
{
    if (token && (m_mens || (token->getDurationFromStart() > 0))) {
        Mensur *layermensuration = new Mensur();
        element->AddChild(layermensuration);
        return layermensuration;
    }

    Mensur *output = (Mensur *)element->FindDescendantByType(ClassId::MENSUR);
    if (!output) {
        output = new Mensur();
        element->AddChild(output);
    }
    return output;
}

/////////////////////////////
//
// HumdrumInput::addSystemClefKeyTimeChange -- Add key or time signature changes
//    for a measure.  The scoreDef element is inserted before the measure in
//    which the change occurs. Presuming all staves have same changes for now.
//

void HumdrumInput::addSystemClefKeyTimeChange(int startline, int endline)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumdrumFile &infile = m_infiles[0];
    hum::HumRegex hre;

    // Keep track of any key and time signature changes for each staff:
    std::vector<hum::HTp> cleftok(ss.size(), NULL);
    std::vector<hum::HTp> keytok(ss.size(), NULL);
    std::vector<hum::HTp> keysigtok(ss.size(), NULL);
    std::vector<hum::HTp> timesigtok(ss.size(), NULL);
    std::vector<hum::HTp> metersigtok(ss.size(), NULL);
    std::vector<hum::HTp> transposetok(ss.size(), NULL);

    bool empty = true;
    bool hasClef = false;
    bool hasTimeSig = false;
    bool hasMeterSig = false;
    bool hasKeySig = false;
    bool hasTranspose = false;

    for (int i = startline; i <= endline; ++i) {
        if (infile[i].isData()) {
            break;
        }
        if (!infile[i].isInterpretation()) {
            continue;
        }
        for (int j = 0; j < infile[i].getFieldCount(); ++j) {
            hum::HTp token = infile.token(i, j);
            int track = token->getTrack();
            int staffindex = m_rkern.at(track);
            if (staffindex < 0) {
                // not a notational spine for a staff
                continue;
            }

            if (hre.search(token, "^\\*M\\d+/\\d+")) {
                timesigtok.at(staffindex) = token;
                empty = false;
                hasTimeSig = true;
            }
            else if (hre.search(token, "^\\*k\\[.*\\]")) {
                keysigtok.at(staffindex) = token;
                empty = false;
                hasKeySig = true;
            }
            else if (token->isClef()) {
                cleftok.at(staffindex) = token;
                empty = false;
                hasClef = true;
            }
            else if (hre.search(token, "^\\*ITrd([-+\\d]+)c([-+\\d]+)")) {
                // update the transposition for notes folling this
                // transposition change.
                m_transpose.at(staffindex) = hum::Convert::transToBase40(*token);
                transposetok.at(staffindex) = token;
                hasTranspose = true;
                empty = false;
            }

            // Meter signature will only be used if immediately following
            // a time signature, so do not set to nonempty by itself.
            if (timesigtok.at(staffindex) && hre.search(token, "^\\*met\\(.*\\)")) {
                metersigtok.at(staffindex) = token;
                hasMeterSig = true;
            }
        }
    }

    if (empty) {
        // No transposition, key or time signature changes.
        return;
    }

    // A score def needs to be added for the clef/key/time sig changes:
    ScoreDef *scoreDef = new ScoreDef();
    m_sections.back()->AddChild(scoreDef);

    // Now need to identifiy if all staves on the system
    // change at the same time, in which case the change
    // is applied at the scoreDef level; otherwise, apply
    // at individual staff level with a staffGrp containing
    // individual changes in separate staffDefs.
    bool allSameTranspose = true;
    bool allSameKeySig = true;
    bool allSameClef = true;
    bool allSameTimeSig = true;
    bool allSameMeterSig = true;

    if (hasClef) {
        if (cleftok[0] == NULL) {
            allSameClef = false;
        }
        else {
            for (int i = 1; i < (int)cleftok.size(); ++i) {
                if (cleftok[i] == NULL) {
                    allSameClef = false;
                    break;
                }
                else if (*cleftok[0] != *cleftok[i]) {
                    allSameClef = false;
                    break;
                }
            }
        }
    }
    else {
        allSameClef = false;
    }
    allSameClef = false; // For always to false for now

    if (hasKeySig) {
        if (keysigtok[0] == NULL) {
            allSameKeySig = false;
        }
        else {
            for (int i = 1; i < (int)keysigtok.size(); ++i) {
                if (keysigtok[i] == NULL) {
                    allSameKeySig = false;
                    break;
                }
                else if (*keysigtok[0] != *keysigtok[i]) {
                    allSameKeySig = false;
                    break;
                }
            }
        }
    }
    else {
        allSameKeySig = false;
    }
    // Don't care about keytok status.

    if (hasTimeSig) {
        if (timesigtok[0] == NULL) {
            allSameTimeSig = false;
        }
        else {
            for (int i = 1; i < (int)timesigtok.size(); ++i) {
                if (timesigtok[i] == NULL) {
                    allSameTimeSig = false;
                    break;
                }
                else if (*timesigtok[0] != *timesigtok[i]) {
                    allSameTimeSig = false;
                    break;
                }
            }
        }
    }
    else {
        allSameTimeSig = false;
    }

    if (hasMeterSig) {
        if (metersigtok[0] == NULL) {
            allSameMeterSig = false;
        }
        else {
            for (int i = 1; i < (int)metersigtok.size(); ++i) {
                if (metersigtok[i] == NULL) {
                    allSameMeterSig = false;
                    break;
                }
                else if (*metersigtok[0] != *metersigtok[i]) {
                    allSameMeterSig = false;
                    break;
                }
            }
        }
    }
    else {
        allSameMeterSig = false;
    }

    if (hasTranspose) {
        if (transposetok[0] == NULL) {
            allSameTranspose = false;
        }
        else {
            for (int i = 1; i < (int)transposetok.size(); ++i) {
                if (transposetok[i] == NULL) {
                    allSameTranspose = false;
                    break;
                }
                else if (*transposetok[0] != *transposetok[i]) {
                    allSameTranspose = false;
                    break;
                }
            }
        }
    }
    else {
        allSameTranspose = false;
    }

    // If there are different active non-zero transpositions
    // then key signatures need to be done individually.
    for (int i = 0; i < (int)m_transpose.size(); ++i) {
        if (m_transpose[i]) {
            allSameKeySig = false;
            break;
        }
    }

    // If the transpositions are different but the key signatures are the
    // same, then make key signatures not the same, since they will be changed
    // by transposition.
    if (allSameKeySig && hasTranspose && !allSameTranspose) {
        allSameKeySig = false;
    }
    if (allSameTimeSig && hasMeterSig && !allSameMeterSig) {
        allSameTimeSig = false;
    }
    if (hasTranspose) {
        // Transposition information cannot be attached to scoreDef, so force onto staffDef:
        allSameTranspose = false;
        allSameKeySig = false;
    }

    bool setAllClef = false;
    bool setAllKeySig = false;
    bool setAllTimeSig = false;

    // First insert changes that affect all staves first.
    if (hasKeySig && allSameKeySig) {
        // eventually allow decoupling of keysig and key.
        setKeySig(-1, scoreDef, *((string *)keysigtok[0]), keysigtok[0], keytok[0], true);
        setAllKeySig = true;
    }

    if (hasClef && allSameClef) {
        // not implemented yet
    }

    if (hasTimeSig && allSameTimeSig) {
        // Disable system-level time signatures for now.
        // setTimeSig(scoreDef, timesigtok[0], metersigtok[0], -1);
        // setAllTimeSig = true;
    }
    if (hasMeterSig && allSameMeterSig) {
        setTimeSig(scoreDef, timesigtok[0], metersigtok[0], -1);
    }

    // Need to add individual staffDefs for changes that do not affect all staves.
    std::vector<bool> needStaffDef(ss.size(), false);
    bool need = false;

    if (!setAllKeySig) {
        for (int i = 0; i < (int)keysigtok.size(); ++i) {
            if (keysigtok[i] != NULL) {
                need = true;
                needStaffDef.at(i) = true;
            }
        }
    }
    if (!setAllClef) {
        for (int i = 0; i < (int)cleftok.size(); ++i) {
            if (cleftok[i] != NULL) {
                need = true;
                needStaffDef.at(i) = true;
            }
        }
    }
    if ((!setAllTimeSig) && (!allSameMeterSig)) {
        for (int i = 0; i < (int)timesigtok.size(); ++i) {
            if (timesigtok[i] != NULL) {
                need = true;
                needStaffDef.at(i) = true;
            }
        }
    }
    for (int i = 0; i < (int)transposetok.size(); ++i) {
        if (transposetok[i] != NULL) {
            need = true;
            needStaffDef.at(i) = true;
        }
    }
    if (!need) {
        return;
    }

    StaffGrp *staffGrp = new StaffGrp();
    scoreDef->AddChild(staffGrp);
    std::vector<StaffDef *> staves(ss.size(), NULL);
    for (int i = 0; i < (int)needStaffDef.size(); ++i) {
        if (!needStaffDef[i]) {
            continue;
        }
        StaffDef *staffDef = new StaffDef();
        staffDef->SetN(i + 1);
        staffGrp->AddChild(staffDef);
        staves[i] = staffDef;
    }

    if (!setAllClef) {
        // add individual clefs to each staff as needed
        for (int i = 0; i < (int)cleftok.size(); ++i) {
            if (cleftok[i] == NULL) {
                continue;
            }
            if (staves[i] == NULL) {
                // should not happen
                continue;
            }
            setClef(staves[i], *cleftok[i], cleftok[i], NULL);
        }
    }

    if (!setAllKeySig) {
        // add individual key signatures to each staff as needed
        for (int i = 0; i < (int)keysigtok.size(); ++i) {
            if (keysigtok[i] == NULL) {
                continue;
            }
            if (staves[i] == NULL) {
                // should not happen
                continue;
            }
            setKeySig(i, staves[i], *((string *)keysigtok[i]), keysigtok[i], keytok[i], true);
        }
    }

    if (!setAllTimeSig) {
        // add individual time signatures to each staff as needed
        for (int i = 0; i < (int)timesigtok.size(); ++i) {
            if (timesigtok[i] == NULL) {
                continue;
            }
            if (staves[i] == NULL) {
                // should not happen
                continue;
            }
            setTimeSig(staves[i], timesigtok[i], metersigtok[i], i);
        }
    }

    // Process any transposition changes:
    for (int i = 0; i < (int)transposetok.size(); ++i) {
        if (transposetok[i] == NULL) {
            continue;
        }
        if (staves[i] == NULL) {
            // should not happen
            continue;
        }
        setTransposition(staves[i], *transposetok[i]);
    }
}

//////////////////////////////
//
// HumdrumInput::insertRepetitionElement --
//

int HumdrumInput::insertRepetitionElement(
    std::vector<std::string> &elements, std::vector<void *> &pointers, std::vector<hum::HTp> tokens, int index)
{
    hum::HTp token = tokens.at(index);
    if (*token != "*rep") {
        // nothing to do
        return index;
    }
    hum::HTp repend = NULL;
    int outindex = index;
    for (int i = index + 1; i < (int)tokens.size(); ++i) {
        if (*tokens[i] == "*Xrep") {
            repend = tokens[i];
            outindex = i;
            break;
        }
    }
    if (!repend) {
        // Cannot find end.  It may be a multi-measure repeat which is not yet handled.
        return index;
    }

    // Figure out if it is a whole measure rest by comparing the timestamps
    // to the width of the layer data.
    hum::HumNum r1time = token->getDurationFromBarline();
    hum::HumNum r2time = repend->getDurationFromBarline();
    hum::HumNum mdur = tokens.back()->getDurationFromBarline();
    if (tokens.back()->isData()) {
        mdur += tokens.back()->getDuration();
    }
    hum::HumNum repdur = r2time - r1time;
    hum::HumNum diff = mdur - repdur;

    if (diff == 0) {
        // Add an mRep to the layer's data and return the index of the *Xrep.
        MRpt *mrpt = new MRpt();
        setLocationId(mrpt, token);
        appendElement(elements, pointers, mrpt);
        mrpt->SetNumVisible(BOOLEAN_false);
        return outindex;
    }

    // Check to see if it is a 1/2 measure repeat
    hum::HumNum ratio = mdur / repdur;
    if (ratio == 2) {
        // The repeat is 1/2 measure long, so insert halfmRpt element.
        HalfmRpt *halfmrpt = new HalfmRpt();
        setLocationId(halfmrpt, token);
        appendElement(elements, pointers, halfmrpt);
        return outindex;
    }

    // The repeat is a beat repeat (presumed, not checking carefully yet).
    BeatRpt *beatrpt = new BeatRpt();
    setLocationId(beatrpt, token);
    setRepeatSlashes(beatrpt, tokens, index);
    appendElement(elements, pointers, beatrpt);
    return outindex;
}

//////////////////////////////
//
// HumdrumInput::setRepeatSlashes -- Check for the next note/rest after
//     the given token and set the slash attribute on the repeat to
//     the power-of-two duration for that rhythm.
//
//

void HumdrumInput::setRepeatSlashes(BeatRpt *repeat, std::vector<hum::HTp> &tokens, int index)
{
    hum::HTp item;
    std::vector<int> repvalues;
    repvalues.reserve(32);

    for (int i = index + 1; i < (int)tokens.size(); ++i) {
        item = tokens.at(i);
        if (*item == "*Xrep") {
            break;
        }
        if (!item->isData()) {
            continue;
        }
        if (item->isNull()) {
            // should not happen, but being careful
            continue;
        }
        if (item->isGrace()) {
            continue;
        }
        hum::HumRegex hre;
        if (!hre.search(item, "(\\d+)")) {
            return;
        }
        double value = hre.getMatchInt(1);
        // remove any tupletness from the rhythm.
        int logvalue = (int)(log(value) / log(2)) - 2;
        repvalues.push_back(logvalue);
    }

    if (repvalues.empty()) {
        // no notes/rests in repetition group for some reason.
        return;
    }

    bool allequal = true;
    for (int i = 1; i < (int)repvalues.size(); ++i) {
        if (repvalues[i] != repvalues[0]) {
            allequal = false;
            break;
        }
    }
    if (!allequal) {
        // set to mixed rhythm case
        repeat->SetSlash(BEATRPT_REND_mixed);
        return;
    }

    switch (repvalues[0]) {
        // default is BEATRPT_REND_1 (one slash)
        case 2: repeat->SetSlash(BEATRPT_REND_2); break;
        case 3: repeat->SetSlash(BEATRPT_REND_3); break;
        case 4: repeat->SetSlash(BEATRPT_REND_4); break;
        case 5: repeat->SetSlash(BEATRPT_REND_5); break;
    }
}

/////////////////////////////
//
// HumdrumInput::insertClefElement -- A clef which starts after the beginning of
// the movement.
//

Clef *HumdrumInput::insertClefElement(
    std::vector<std::string> &elements, std::vector<void *> &pointers, hum::HTp token, hum::HTp lastnote)
{
    Clef *clef = new Clef();

    bool sameas = false;
    hum::HumNum clefpos = -1;
    if (lastnote) {
        clefpos = token->getDurationFromBarline();
        hum::HumNum notepos = lastnote->getDurationFromBarline();
        hum::HumNum duration = hum::Convert::recipToDuration(lastnote);
        if (notepos + duration != clefpos) {
            sameas = true;
        }
    }

    // Store all of the clefs for later sameas analysis:
    m_clef_buffer.push_back(std::make_tuple(sameas, clefpos, clef));
    if (sameas) {
        // make 100% transparent red in case sameas method changes:
        // See issue https://github.com/humdrum-tools/verovio-humdrum-viewer/issues/546
        // clef->SetColor("#ff000000");
        // appendTypeTag(clef, "sameas");
    }

    setClefColorOrEditorial(token, clef, elements, pointers);
    setLocationId(clef, token);

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    ss.at(m_currentstaff - 1).last_clef = *token;

    setClefBasicShape(clef, *token);
    setClefStaffLine(clef, *token);
    setClefOctaveDisplacement(clef, *token);
    checkForClefStyling(clef, token);
    return clef;
}

//////////////////////////////
//
// HumdrumInput::setClefBasicShape --
//     *clefG = G clef
//     *clefF = F clef
//     *clefC = C clef
//     *clefX = percussion clef

void HumdrumInput::setClefBasicShape(Clef *clef, const std::string &tok)
{
    if (tok.find("clefG") != std::string::npos) {
        clef->SetShape(CLEFSHAPE_G);
    }
    else if (tok.find("clefF") != std::string::npos) {
        clef->SetShape(CLEFSHAPE_F);
    }
    else if (tok.find("clefC") != std::string::npos) {
        clef->SetShape(CLEFSHAPE_C);
    }
}

//////////////////////////////
//
// HumdrumInput::setClefOctaveDisplacement -- Add 8 or 15 above/below clef.
//    *clefGv2 == v means play one octave lower than written
//    *clefG^2 == ^ means play one octave higher than written
//    *clefGvv2 == vv means play two octaves lower than written
//    *clefG^^2 == ^^ means play two octaves higher than written
//

void HumdrumInput::setClefOctaveDisplacement(Clef *clef, const std::string &tok)
{

    if (tok.find("vv") != std::string::npos) {
        clef->SetDis(OCTAVE_DIS_15);
        clef->SetDisPlace(STAFFREL_basic_below);
    }
    else if (tok.find("v") != std::string::npos) {
        clef->SetDis(OCTAVE_DIS_8);
        clef->SetDisPlace(STAFFREL_basic_below);
    }
    else if (tok.find("^^") != std::string::npos) {
        clef->SetDis(OCTAVE_DIS_15);
        clef->SetDisPlace(STAFFREL_basic_above);
    }
    else if (tok.find("^") != std::string::npos) {
        clef->SetDis(OCTAVE_DIS_8);
        clef->SetDisPlace(STAFFREL_basic_above);
    }
}

//////////////////////////////
//
// HumdrumInput::setClefColorOrEditorial --
//

void HumdrumInput::setClefColorOrEditorial(
    hum::HTp token, Clef *clef, std::vector<std::string> &elements, std::vector<void *> &pointers, bool append)
{
    if (!token) {
        return;
    }
    if (!clef) {
        return;
    }

    bool iseditorial = getBooleanParameter(token, "CL", "ed");
    std::string color = getStringParameter(token, "CL", "color");

    if (iseditorial) {
        if (append) {
            // Initial clef cannot yet be supplied.
            Supplied *supplied = new Supplied();
            appendElement(supplied, clef);
            appendElement(elements, pointers, supplied);
            if (color.empty()) {
                clef->SetColor("#aaa"); // hard-code to gray by default for now
            }
            else {
                clef->SetColor(color);
            }
            appendTypeTag(clef, "editorial");
        }
    }
    else {
        if (append) {
            appendElement(elements, pointers, clef);
        }
        if (!color.empty()) {
            clef->SetColor(color);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForClefStyling --
//

void HumdrumInput::checkForClefStyling(Clef *clef, hum::HTp token)
{
    if (!clef) {
        return;
    }
    if (!token) {
        return;
    }

    bool gg = getBooleanParameter(token, "CL", "gg");
    if (gg) {
        clef->SetGlyphName("gClef8vbOld");
        clef->SetGlyphAuth("smufl");
        return;
    }

    std::string smufl = getStringParameter(token, "CL", "smufl");

    if ((smufl != "") && (smufl != "true") && (smufl != "false")) {
        // Use a non-default glyph from SMuFL:
        clef->SetGlyphName(smufl);
        clef->SetGlyphAuth("smufl");
        return;
    }

    // Check for displaying clef in mensural or chant style:
    bool mens = getBooleanParameter(token, "CL", "mens");

    if (mens) {
        // Use a (Petrucci) mensural shape (in modern notation)
        if (token->compare(0, 6, "*clefG") == 0) {
            clef->SetGlyphName("mensuralGclefPetrucci");
            clef->SetGlyphAuth("smufl");
        }
        else if (token->compare(0, 6, "*clefF") == 0) {
            clef->SetGlyphName("mensuralFclefPetrucci");
            clef->SetGlyphAuth("smufl");
        }
        else if (*token == "*clefC1") {
            clef->SetGlyphName("mensuralCclefPetrucciPosLowest");
            clef->SetGlyphAuth("smufl");
        }
        else if (*token == "*clefC2") {
            clef->SetGlyphName("mensuralCclefPetrucciPosLow");
            clef->SetGlyphAuth("smufl");
        }
        else if (*token == "*clefC3") {
            clef->SetGlyphName("mensuralCclefPetrucciPosMiddle");
            clef->SetGlyphAuth("smufl");
        }
        else if (*token == "*clefC4") {
            clef->SetGlyphName("mensuralCclefPetrucciPosHigh");
            clef->SetGlyphAuth("smufl");
        }
        else if (*token == "*clefC5") {
            clef->SetGlyphName("mensuralCclefPetrucciPosHighest");
            clef->SetGlyphAuth("smufl");
        }
        return;
    }

    bool chant = getBooleanParameter(token, "CL", "chant");
    if (chant) {
        // Use a chant-style clef (in modern notation)
        if (token->compare(0, 6, "*clefC") == 0) {
            clef->SetGlyphName("chantCclef");
            clef->SetGlyphAuth("smufl");
        }
        else if (token->compare(0, 6, "*clefF") == 0) {
            clef->SetGlyphName("chantFclef");
            clef->SetGlyphAuth("smufl");
        }
        else if (token->compare(0, 6, "*clefG") == 0) {
            clef->SetGlyphName("mensuralGclef");
            clef->SetGlyphAuth("smufl");
        }
        return;
    }

    // Check for display of modern clefs (in **mens music typically)
    bool cmn = getBooleanParameter(token, "CL", "cmn");
    if (cmn) {
        // Use a chant-style clef (in modern notation)
        if (token->compare(0, 6, "*clefC") == 0) {
            clef->SetGlyphName("cClef");
            clef->SetGlyphAuth("smufl");
        }
        else if (token->compare(0, 6, "*clefF") == 0) {
            clef->SetGlyphName("fClef");
            clef->SetGlyphAuth("smufl");
        }
        else if (token->compare(0, 6, "*clefG") == 0) {
            clef->SetGlyphName("gClef");
            clef->SetGlyphAuth("smufl");
        }
        return;
    }
}

//////////////////////////////
//
// HumdrumInput::getBooleanParameter --
//

bool HumdrumInput::getBooleanParameter(hum::HTp token, const std::string &category, const std::string &key)
{
    int lcount = token->getLinkedParameterSetCount();
    for (int i = 0; i < lcount; ++i) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(i);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        std::string pkey;
        // std::string value;
        for (int i = 0; i < hps->getCount(); ++i) {
            pkey = hps->getParameterName(i);
            // value = hps->getParameterValue(i);
            if (pkey == key) {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::getStringParameter --
//

std::string HumdrumInput::getStringParameter(hum::HTp token, const std::string &category, const std::string &key)
{
    int lcount = token->getLinkedParameterSetCount();
    for (int i = 0; i < lcount; ++i) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(i);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != category) {
            continue;
        }
        std::string pkey;
        std::string value;
        for (int i = 0; i < hps->getCount(); ++i) {
            pkey = hps->getParameterName(i);
            if (pkey == key) {
                value = hps->getParameterValue(i);
                return value;
            }
        }
    }
    return "";
}

//////////////////////////////
//
// HumdrumInput::storeBreaksec -- Look for cases where sub-beams are broken.
//

void HumdrumInput::storeBreaksec(
    std::vector<int> &beamstate, std::vector<int> &beamnum, const std::vector<hum::HTp> &layerdata, bool grace)
{

    std::vector<std::vector<int>> beamednotes;
    int bnum = 0;
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        if (!beamnum[i]) {
            // not in a beam
            continue;
        }
        if (!layerdata[i]->isData()) {
            // not a note or rest in the beam
            continue;
        }
        if (layerdata[i]->isNull()) {
            // shouldn't happen, but just in case.
            continue;
        }
        if ((!grace) && layerdata[i]->isGrace()) {
            // ignore grace notes in this analysis
            continue;
        }
        if (grace && !layerdata[i]->isGrace()) {
            // ignore grace notes in this analysis
            continue;
        }
        if (bnum != beamnum[i]) {
            // create a new list of notes.
            beamednotes.resize(beamednotes.size() + 1);
            bnum = beamnum[i];
        }
        beamednotes.back().push_back(i);
    }

    for (int i = 0; i < (int)beamednotes.size(); ++i) {
        for (int j = 1; j < (int)beamednotes[i].size() - 1; j++) {
            int index1 = beamednotes[i][j - 1];
            int index2 = beamednotes[i][j];
            int index3 = beamednotes[i][j + 1];
            int bcount1 = beamstate[index1];
            int bcount2 = beamstate[index2];
            int bcount3 = beamstate[index3];
            if ((bcount2 < bcount1) && (bcount2 < bcount3)) {
                // mark a breaksec for the given note/chord/rest.
                layerdata[index2]->setValue("", "auto", "breaksec", to_string(bcount2));
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeLayerBeams --
//

void HumdrumInput::analyzeLayerBeams(
    std::vector<int> &beamnum, std::vector<int> &gbeamnum, const std::vector<hum::HTp> &layerdata)
{

    std::vector<int> beamstate(layerdata.size(), 0);
    std::vector<int> gbeamstate(layerdata.size(), 0); // for grace notes
    int negativeQ = 0;
    int gnegativeQ = 0;
    int lastbeamstate = 0;
    int lastgbeamstate = 0;

    int i;
    for (i = 0; i < (int)beamstate.size(); ++i) {
        if (!layerdata[i]->isData()) {
            beamstate[i] = lastbeamstate;
            gbeamstate[i] = lastgbeamstate;
            continue;
        }
        if (layerdata[i]->isNull()) {
            // shouldn't get to this state
            beamstate[i] = lastbeamstate;
            gbeamstate[i] = lastgbeamstate;
            continue;
        }
        if (layerdata[i]->isGrace()) {
            gbeamstate[i] = characterCount(*layerdata[i], 'L');
            gbeamstate[i] -= characterCount(*layerdata[i], 'J');
            lastgbeamstate = gbeamstate[i];
        }
        else {
            int Lcount = characterCount(*layerdata[i], 'L');
            int Jcount = characterCount(*layerdata[i], 'J');
            bool beamSpanStart = layerdata[i]->getValueBool("auto", "beamSpanStart");
            // bool beamSpanEnd = layerdata[i]->getValueBool("auto", "beamSpanEnd");
            bool inBeamSpan = layerdata[i]->getValueBool("auto", "inBeamSpan");
            if (!inBeamSpan) {
                beamstate[i] = Lcount;
                beamstate[i] -= Jcount;
                lastbeamstate = beamstate[i];
            }
            else if (beamSpanStart) {
                m_beamSpanStartDatabase.push_back(layerdata[i]);
                beamstate[i] = lastbeamstate;
                gbeamstate[i] = lastgbeamstate;
                continue;
            }
            else {
                beamstate[i] = lastbeamstate;
                gbeamstate[i] = lastgbeamstate;
            }
        }
        if (i > 0) {
            beamstate[i] += beamstate[i - 1];
            gbeamstate[i] += gbeamstate[i - 1];
            lastbeamstate = beamstate[i];
            lastgbeamstate = gbeamstate[i];
        }
    }

    // Adjust the beam states if there are any negative values in it:
    int min = 0;
    for (int i = 0; i < (int)beamstate.size(); ++i) {
        if (beamstate[i] < min) {
            min = beamstate[i];
        }
    }
    if (min < 0) {
        for (int i = 0; i < (int)beamstate.size(); ++i) {
            beamstate[i] -= min;
        }
    }

    if (m_debug) {
        for (int i = 0; i < (int)beamstate.size(); ++i) {
            cerr << layerdata[i] << "(" << beamstate[i] << ")  ";
        }
        cerr << endl;
    }

    // int beamstartindex = -1;
    // int beamendindex = -1;
    if (beamstate.size() > 0) {
        if (beamstate.back() > 0) {
            // Extra beam start(s) at the end of the measure.  Remove all positive numbers at the back of the
            // beamstate list until a zero is found (for the end of an in-measure beam end).
            for (int i = (int)beamstate.size() - 1; i >= 0; i--) {
                if (beamstate[i] == 0) {
                    // beamstartindex = i + 1;
                    break;
                }
                beamstate[i] = 0;
            }
        }
        else if (beamstate.back() < 0) {
            // Extra beam ends at the start of the measure.
            // After the first non-zero (and hopefully negative value) in the beamstate,
            // subtract the last value in beam state (which is hopefully the same as the
            // first non-zero value.
            bool nonzero = false;
            for (int i = 0; i < (int)beamstate.size(); ++i) {
                if (!nonzero) {
                    if (beamstate[i] == 0) {
                        continue;
                    }
                    else {
                        nonzero = true;
                        if (beamstate[i] != beamstate.back()) {
                            // complicated beamspan case that is not yet handled.
                            break;
                        }
                        // beamendindex = i;
                    }
                }
                beamstate[i] -= beamstate.back();
            }
        }
    }

    negativeQ = 0;
    for (int i = 0; i < (int)beamstate.size(); ++i) {
        if (beamstate[i] < 0) {
            negativeQ = 1;
            break;
        }
    }

    gnegativeQ = 0;
    for (int i = 0; i < (int)gbeamstate.size(); ++i) {
        if (gbeamstate[i] < 0) {
            gnegativeQ = 1;
            break;
        }
    }

    // if (beamstartindex >= 0) {
    //     layerdata.at(beamstartindex)->setValue("auto", "beamSpanStart", 1);
    //     // Store measure that the beamSpan starts in:
    //     m_beamSpanStartDatabase[layerdata.at(beamstartindex)] = m_measure;
    // }
    // if (beamendindex >= 0) {
    //     layerdata.at(beamendindex)->setValue("auto", "beamSpanEnd", 1);
    //     insertBeamSpan(layerdata.at(beamendindex));
    // }

    // Convert to beam enumerations.  Beamstates are nonzero for the
    // notes in a beam, but the last one is zero.
    int bcounter = 1;
    beamnum.resize(beamstate.size());
    std::fill(beamnum.begin(), beamnum.end(), 0);
    if (beamstate[0]) {
        beamnum[0] = bcounter;
    }
    else {
        beamnum[0] = 0;
    }
    for (i = 1; i < (int)beamstate.size(); ++i) {
        if (beamstate[i]) {
            beamnum[i] = bcounter;
        }
        if ((!beamstate[i]) && beamstate[i - 1]) {
            beamnum[i] = bcounter++;
        }
    }

    bcounter = 1;
    gbeamnum.resize(beamstate.size());
    std::fill(gbeamnum.begin(), gbeamnum.end(), 0);
    if (gbeamstate[0]) {
        gbeamnum[0] = bcounter;
    }
    else {
        gbeamnum[0] = 0;
    }
    for (i = 1; i < (int)gbeamstate.size(); ++i) {
        if (gbeamstate[i]) {
            gbeamnum[i] = bcounter;
        }
        if ((!gbeamstate[i]) && gbeamstate[i - 1]) {
            gbeamnum[i] = bcounter++;
        }
    }

    if (negativeQ || (beamstate.back() != 0)) {
        // something wrong with the beaming, either incorrect or
        // the beaming crosses a barline or layer.  Don't try to
        // beam anything.
        std::fill(beamstate.begin(), beamstate.end(), 0);
        std::fill(beamnum.begin(), beamnum.end(), 0);
    }

    if (gnegativeQ || (gbeamstate.back() != 0)) {
        // something wrong with the graceote beaming, either incorrect or
        // the beaming crosses a barline or layer.  Don't try to
        // beam anything.
        std::fill(gbeamstate.begin(), gbeamstate.end(), 0);
        std::fill(gbeamnum.begin(), gbeamnum.end(), 0);
    }

    storeBreaksec(beamstate, beamnum, layerdata);
    storeBreaksec(gbeamstate, gbeamnum, layerdata, true);
}

//////////////////////////////
//
// HumdrumInput::storeBeamSpansInStartingMeasure --
//

void HumdrumInput::storeBeamSpansInStartingMeasure()
{
    for (int i = 0; i < (int)m_beamSpanStartDatabase.size(); ++i) {
        this->insertBeamSpan(m_beamSpanStartDatabase[i]);
    }
    m_beamSpanStartDatabase.clear();
}

//////////////////////////////
//
// HumdrumInput::insertBeamSpan -- Also deal with hanging beams.
//

void HumdrumInput::insertBeamSpan(hum::HTp token)
{
    if (!token) {
        return;
    }
    bool hangingQ = token->getValueBool("auto", "hangingBeam");
    if (hangingQ) {
        // Not dealing with hanging beams for now.
        // See https://github.com/rism-digital/verovio/issues/2786
        return;
    }
    // Ignore grace note beamspans for now:
    if (token->find("q") != std::string::npos) {
        return;
    }
    bool spanStart = token->getValueBool("auto", "beamSpanStart");
    if (!spanStart) {
        return;
    }
    hum::HTp etok = token->getValueHTp("auto", "beamEndId");
    if (!etok) {
        return;
    }

    BeamSpan *beamspan = new BeamSpan();

    std::string startid = getDataTokenId(token);
    std::string endid = getDataTokenId(etok);

    beamspan->SetStartid("#" + startid);
    beamspan->SetEndid("#" + endid);

    setBeamSpanPlist(beamspan, token, etok);

    addChildMeasureOrSection(beamspan);
}

//////////////////////////////
//
// HumdrumInput::setBeamSpanPlist -- Not fully generalized (higher layer number
//    to lower one is allowed but not the other way around).  Cross-staff needs
//    to be handled separatly, probably by marking individual notes in beamSpan.
//

void HumdrumInput::setBeamSpanPlist(BeamSpan *beamspan, hum::HTp starttok, hum::HTp endtok)
{
    std::vector<hum::HTp> tokens;
    hum::HTp current = starttok;
    if (current) {
        tokens.push_back(current);
    }
    int endline = endtok->getLineIndex();

    current = current->getNextToken();
    while (current) {
        if (current == endtok) {
            tokens.push_back(current);
            break;
        }
        int cline = current->getLineIndex();
        if (cline > endline) {
            // Something bad happened
            break;
        }
        if (!current->isData()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isNull()) {
            current = current->getNextToken();
            continue;
        }
        tokens.push_back(current);
        current = current->getNextToken();
    }

    for (int i = 0; i < (int)tokens.size(); ++i) {
        std::string idvalue = getDataTokenId(tokens[i]);
        beamspan->AddRef("#" + idvalue);
    }
}

//////////////////////////////
//
// HumdrumInput::getDataTokenId -- Decide if note, chord, or rest.
//

std::string HumdrumInput::getDataTokenId(hum::HTp token)
{
    if (token->isChord()) {
        return getLocationId("chord", token);
    }
    else if (token->isRest()) {
        return getLocationId("rest", token);
    }
    else {
        return getLocationId("note", token);
    }
}

//////////////////////////////
//
// HumdrumInput::shouldHideBeamBracket --
//

bool HumdrumInput::shouldHideBeamBracket(
    const std::vector<humaux::HumdrumBeamAndTuplet> &tgs, std::vector<hum::HTp> &layerdata, int layerindex)
{
    hum::HTp starttok = layerdata[layerindex];
    if (starttok->find("L") == std::string::npos) {
        return false;
    }
    bool beamedge = false;
    int targettup = tgs.at(layerindex).tupletstart;
    int i = layerindex + 1;
    hum::HTp endtok = NULL;
    while (i < (int)layerdata.size()) {
        if (tgs.at(i).tupletend == targettup) {
            endtok = layerdata[i];
            break;
        }
        if (tgs.at(i).beamstart) {
            beamedge = true;
        }
        if (tgs.at(i).beamend) {
            beamedge = true;
        }
        i++;
    }
    if (!endtok) {
        return false;
    }
    if (beamedge) {
        return false;
    }
    if (endtok->find("J") == std::string::npos) {
        return false;
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::insertTuplet --
//

void HumdrumInput::insertTuplet(std::vector<std::string> &elements, std::vector<void *> &pointers,
    const std::vector<humaux::HumdrumBeamAndTuplet> &tgs, std::vector<hum::HTp> layerdata, int layerindex,
    bool suppressTupletNumber, bool suppressBracketTuplet)
{

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    hum::HTp token = layerdata[layerindex];
    const humaux::HumdrumBeamAndTuplet &tg = tgs.at(layerindex);

    Tuplet *tuplet = new Tuplet();
    setTupletLocationId(tuplet, tgs, layerdata, layerindex);
    appendElement(elements, pointers, tuplet);
    elements.push_back("tuplet");
    pointers.push_back((void *)tuplet);

    int staffindex = m_rkern[token->getTrack()];
    int placement = 0;

    int rstaffindex = (int)m_fbstaff.size() - staffindex - 1;
    if (m_fbstaff[rstaffindex]) {
        placement = -m_fbstates.at(rstaffindex);
    }

    if (hasAboveParameter(layerdata[layerindex], "TUP")) {
        placement = +1;
    }
    else if (hasBelowParameter(layerdata[layerindex], "TUP")) {
        placement = -1;
    }
    // add head and stem placement in the future.
    if (placement) {
        switch (placement) {
            case -1: tuplet->SetBracketPlace(STAFFREL_basic_below); break;
            case +1: tuplet->SetBracketPlace(STAFFREL_basic_above); break;
        }
    }
    if (ss[staffindex].verse) {
        // If the music contains lyrics, force the tuplet above the staff.
        tuplet->SetBracketPlace(STAFFREL_basic_above);
    }

    double scale = tg.numscale;
    tuplet->SetNum(tg.num * scale);
    tuplet->SetNumbase(tg.numbase * scale);
    if (suppressBracketTuplet || suppressTupletNumber) {
        tuplet->SetBracketVisible(BOOLEAN_false);
    }
    if (shouldHideBeamBracket(tgs, layerdata, layerindex)) {
        tuplet->SetBracketVisible(BOOLEAN_false);
    }

    // local control of brackets
    bool xbr = hasLayoutParameter(token, "TUP", "xbr");
    bool br = hasLayoutParameter(token, "TUP", "br");
    if (xbr) {
        tuplet->SetBracketVisible(BOOLEAN_false);
    }
    if (br) {
        tuplet->SetBracketVisible(BOOLEAN_true);
    }

    // Brackets will be displayed automatically, so don't turn on:
    // else {
    //     if (tg.bracket) {
    //         tuplet->SetBracketVisible(BOOLEAN_true);
    //     }
    //     else {
    //         tuplet->SetBracketVisible(BOOLEAN_false);
    //     }
    // }
    if (suppressTupletNumber) {
        // Number is visible by default, so only hide
        // if explicitly requested:
        tuplet->SetNumVisible(BOOLEAN_false);
    }
    hum::HumNum base = tg.numbase;
    // if (!base.isPowerOfTwo()) {
    //     tuplet->SetNumFormat(tupletVis_NUMFORMAT_ratio);
    // }
    // else {
    tuplet->SetNumFormat(tupletVis_NUMFORMAT_count);
    // }
    m_tupletscaling = tg.num;
    m_tupletscaling /= tg.numbase;
}

//////////////////////////////
//
// HumdrumInput::insertBeam --
//

Beam *HumdrumInput::insertBeam(
    std::vector<std::string> &elements, std::vector<void *> &pointers, const humaux::HumdrumBeamAndTuplet &tg)
{
    Beam *beam = new Beam();
    appendElement(elements, pointers, beam);
    elements.push_back("beam");
    pointers.push_back((void *)beam);
    return beam;
}

//////////////////////////////
//
// HumdrumInput::insertGBeam --
//

Beam *HumdrumInput::insertGBeam(
    std::vector<std::string> &elements, std::vector<void *> &pointers, const humaux::HumdrumBeamAndTuplet &tg)
{
    Beam *gbeam = new Beam();
    appendElement(elements, pointers, gbeam);
    elements.push_back("gbeam");
    pointers.push_back((void *)gbeam);
    return gbeam;
}

//////////////////////////////
//
// HumdrumInput::removeBeam --
//

void HumdrumInput::removeBeam(std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    if (elements.back() != "beam") {
        cerr << "ERROR REMOVING BEAM" << endl;
        cerr << "ELEMENT STACK:" << endl;
        for (int i = (int)elements.size() - 1; i >= 0; i--) {
            cerr << i << ":\t" << elements[i] << endl;
        }
        return;
    }
    popElementStack(elements, pointers);
}

//////////////////////////////
//
// HumdrumInput::removeGBeam --
//

void HumdrumInput::removeGBeam(std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    if (elements.back() != "gbeam") {
        cerr << "ERROR REMOVING GBEAM" << endl;
        cerr << "ELEMENT STACK:" << endl;
        for (int i = (int)elements.size() - 1; i >= 0; i--) {
            cerr << i << ":\t" << elements[i] << endl;
        }
        return;
    }
    popElementStack(elements, pointers);
}

//////////////////////////////
//
// HumdrumInput::removeTuplet --
//

void HumdrumInput::removeTuplet(std::vector<std::string> &elements, std::vector<void *> &pointers)
{
    if (elements.back() != "tuplet") {
        cerr << "ERROR REMOVING TUPLET" << endl;
        cerr << "ELEMENT BACK IS " << elements.back() << endl;
        cerr << "ELEMENT STACK:" << endl;
        for (int i = (int)elements.size() - 1; i >= 0; i--) {
            cerr << i << ":\t" << elements[i] << endl;
        }
        return;
    }
    popElementStack(elements, pointers);

    // Need to fix this when nested tuplets are allowed:
    m_tupletscaling = 1;
}

//////////////////////////////
//
// HumdrumInput::prepareBeamAndTupletGroups -- Calculate beam and tuplet
//     groupings for a layer.
//

void HumdrumInput::prepareBeamAndTupletGroups(
    std::vector<humaux::HumdrumBeamAndTuplet> &tgs, const std::vector<hum::HTp> &layerdata)
{
    std::vector<int> beamnum;
    std::vector<int> gbeamnum;
    analyzeLayerBeams(beamnum, gbeamnum, layerdata);
    tgs.clear();

    // duritems == a list of items in the layer which have duration.
    // Grace notes, barlines, interpretations, local comments, global comments,
    // etc. are filtered out for the analysis.
    std::vector<hum::HTp> duritems;

    // indexmapping == maping from a duritem index to a layerdata index.
    std::vector<int> indexmapping;

    // indexmapping2 == mapping from a layerdata index to a duritem index,
    // with -1 meaning no mapping.
    std::vector<int> indexmapping2;

    // durbeamnum == beam numbers for durational items only.
    std::vector<int> durbeamnum;

    // Extract a list of the layer items that have duration:
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        if (!layerdata[i]->isData()) {
            indexmapping2.push_back(-1);
            continue;
        }
        if (layerdata[i]->isNull()) {
            indexmapping2.push_back(-1);
            continue;
        }
        if (layerdata[i]->isGrace()) {
            indexmapping2.push_back(-1);
            continue;
        }
        // don't consider notes without durations
        hum::HumNum dur = hum::Convert::recipToDuration(layerdata[i]);
        if (dur == 0) {
            indexmapping2.push_back(-1);
            continue;
        }
        indexmapping.push_back(i);
        indexmapping2.push_back((int)indexmapping.size() - 1);
        duritems.push_back(layerdata[i]);
        durbeamnum.push_back(beamnum[i]);
    }

    // poweroftwo == keeps track whether durations are based on a power
    // (non-tuplet) or not (tuplet).  Notes/rests with false poweroftwo
    // will be grouped into tuplets.
    std::vector<bool> poweroftwo(duritems.size());
    bool hastupletQ = false;
    std::vector<hum::HumNum> dotlessdur(duritems.size());
    for (int i = 0; i < (int)duritems.size(); ++i) {
        hum::HumNum duration = hum::Convert::recipToDurationNoDots(*duritems[i]);
        dotlessdur[i] = duration / 4;
        poweroftwo[i] = duration.isPowerOfTwo();
        hastupletQ |= !poweroftwo[i];
    }

    // Count the number of beams.  The durbeamnum std::vector contains a list
    // of beam numbers starting from 1 (or 0 if a note/rest has no beam).
    int beamcount = 0;
    for (int i = 0; i < (int)durbeamnum.size(); ++i) {
        if (durbeamnum[i] > beamcount) {
            beamcount = durbeamnum[i];
        }
    }

    // beamstarts and beamends are lists of the starting and ending
    // index for beams of duration items in the layer.  The index is
    // into the durlist std::vector (list of items which posses duration).
    std::vector<int> beamstarts(beamcount, -1);
    std::vector<int> beamends(beamcount, 0);
    for (int i = 0; i < (int)durbeamnum.size(); ++i) {
        if (durbeamnum[i]) {
            if (beamstarts[durbeamnum[i] - 1] < 0) {
                beamstarts[durbeamnum[i] - 1] = i;
            }
            beamends[durbeamnum[i] - 1] = i;
        }
    }

    // beamstartboolean == starting of a beam on a particular note
    // beamendboolean == ending of a beam on a particular note
    std::vector<int> beamstartboolean(durbeamnum.size(), 0);
    std::vector<int> beamendboolean(durbeamnum.size(), 0);
    for (int i = 0; i < (int)beamstarts.size(); ++i) {
        beamstartboolean[beamstarts[i]] = i + 1;
        beamendboolean[beamends[i]] = i + 1;
    }

    // Calculate grace note beam starts and ends.
    // Presuming no clef changes, etc. found between notes in
    // a gracenote beam.  Generalize further if so.
    // gbeamstart == boolean for starting of a grace note beam
    // gbeamend == boolean ending of a grace note beam
    std::vector<int> gbeamstart(layerdata.size(), 0);
    std::vector<int> gbeamend(layerdata.size(), 0);
    std::vector<int> gstate(layerdata.size(), 0);
    for (int i = 0; i < (int)gbeamnum.size(); ++i) {
        if (!gbeamnum[i]) {
            continue;
        }
        else if (gstate[gbeamnum[i]]) {
            continue;
        }
        else {
            gstate[gbeamnum[i]] = 1;
            gbeamstart[i] = gbeamnum[i];
        }
    }

    std::fill(gstate.begin(), gstate.end(), 0);
    for (int i = (int)gbeamnum.size() - 1; i >= 0; i--) {
        if (!gbeamnum[i]) {
            continue;
        }
        else if (gstate[gbeamnum[i]]) {
            continue;
        }
        else {
            gstate[gbeamnum[i]] = 1;
            gbeamend[i] = gbeamnum[i];
        }
    }

    // tgs may not be completly filled in if there are no tuplets. Check on this later.
    if (!hastupletQ) {
        tgs.resize(layerdata.size());
        for (int i = 0; i < (int)layerdata.size(); ++i) {
            tgs.at(i).token = layerdata[i];
            tgs.at(i).gbeamstart = gbeamstart.at(i);
            tgs.at(i).gbeamend = gbeamend.at(i);
            if (indexmapping2[i] < 0) {
                continue;
            }
            tgs.at(i).beamstart = beamstartboolean.at(indexmapping2.at(i));
            tgs.at(i).beamend = beamendboolean.at(indexmapping2.at(i));
            if (tgs.at(i).beamstart) {
                tgs.at(i).token->setValue("auto", "beamstart", tgs.at(i).beamstart);
            }
            if (tgs.at(i).beamend) {
                tgs.at(i).token->setValue("auto", "beamend", tgs.at(i).beamend);
            }
        }
        return;
    }

    // durationwithdots == full duration of the note/rest including augmentation dots.
    std::vector<hum::HumNum> durationwithdots(duritems.size());

    // dursum = a cumulative sum of the full durs, starting at 0 for
    // the first index.
    std::vector<hum::HumNum> dursum(duritems.size());

    hum::HumNum sum = 0;
    std::vector<int> twocounttop(dotlessdur.size(), 0);
    std::vector<int> twocountbot(dotlessdur.size(), 0);
    for (int i = 0; i < (int)dotlessdur.size(); ++i) {
        durationwithdots[i] = hum::Convert::recipToDuration(*duritems[i]);
        dursum[i] = sum;
        sum += durationwithdots[i];
    }

    // beamdur = a list of the durations for each beam.
    std::vector<hum::HumNum> beamdur(beamstarts.size());
    for (int i = 0; i < (int)beamdur.size(); ++i) {
        beamdur[i] = dursum[beamends[i]] - dursum[beamstarts[i]] + durationwithdots[beamends[i]];
    }

    // beampowdot == the number of augmentation dots on a power of two for
    // the duration of the beam.  -1 means could not be made power of two with
    // dots.
    std::vector<int> beampowdot(beamstarts.size(), -1);
    for (int i = 0; i < (int)beampowdot.size(); ++i) {
        beampowdot[i] = getDotPowerOfTwo(beamdur[i]);
    }

    std::vector<bool> binarybeams(beamstarts.size(), false);
    for (int i = 0; i < (int)binarybeams.size(); ++i) {
        if (poweroftwo[beamstarts[i]]) {
            binarybeams[i] = true;
        }
    }

    // Assume that tuplet beams that can fit into a power of two will form
    // a tuplet group.  Perhaps bias towards beampowdot being 0, and try to
    // beam groups to include non-beamed tuplets into lower powdots.
    // Should check that the factors of notes in the beam group all match...
    std::vector<int> tupletgroups(poweroftwo.size(), 0);

    // durforce: boolean for if a tuplet has been forced to be
    // be broken on the current note.  This is used to prevent
    // merging of the break when trying to merge tuplets.
    std::vector<bool> durforce(poweroftwo.size(), false);

    // tupletbracket == boolean for if the tuplet group requires a bracket.
    // It will require a bracket if they are not all enclosed in a beam.
    std::vector<int> tupletbracket(poweroftwo.size(), -1);
    int tupletnum = 1;

    // adjustcount == Adjusted tuplet number by tuplet group, probably not needed
    // anymore since tuplet scaling is done independently at the end of this
    // function.
    std::vector<int> adjustcount;

    hum::HumNum tupletdur = 0;
    int tupletcount = 0;
    int samedurtup = true;

    for (int i = 0; i < (int)beampowdot.size(); ++i) {
        if (binarybeams[i]) {
            continue;
        }
        bool ingroup = false;
        if (beampowdot[i] >= 0) {
            for (int j = beamstarts[i]; j <= beamends[i]; ++j) {

                // may have to deal with dotted triplets (that appear to be powers of two)
                if (poweroftwo[j]) {
                    if (ingroup) {
                        ingroup = false;
                        tupletnum++;
                    }
                    tupletbracket[j] = 0;
                    continue;
                }
                ingroup = true;
                tupletgroups.at(j) = tupletnum;
                if (tupletcount == 0) {
                    samedurtup = true;
                    tupletdur = durationwithdots[j];
                }
                else if (tupletdur != durationwithdots[j]) {
                    samedurtup = false;
                }
                tupletcount++;

                tupletbracket[j] = 0;
            }
            if (samedurtup) {
                if (tupletnum >= (int)adjustcount.size()) {
                    int oldsize = (int)adjustcount.size();
                    adjustcount.resize(tupletnum + 1);
                    for (int z = (int)oldsize; z < (int)adjustcount.size(); ++z) {
                        adjustcount[z] = 0;
                    }
                }
                adjustcount[tupletnum] = tupletcount;
            }
            tupletnum++;
            tupletcount = 0;
        }
    }

    int tcorrection = 0;
    for (int i = 0; i < (int)tupletgroups.size(); ++i) {
        if (checkForTupletForcedBreak(duritems, i)) {
            tcorrection++;
        }
        if (tupletgroups.at(i)) {
            tupletgroups.at(i) += tcorrection;
        }
    }
    if (tcorrection) {
        // invalidate adjustcount
        for (int i = 0; i < (int)adjustcount.size(); ++i) {
            adjustcount[i] = 0;
        }
    }

    std::vector<bool> beamstarts2(poweroftwo.size(), false);
    for (int i = 0; i < (int)beamstarts.size(); ++i) {
        beamstarts2[beamstarts[i]] = true;
    }

    std::vector<bool> beamends2(poweroftwo.size(), false);
    for (int i = 0; i < (int)beamends.size(); ++i) {
        beamends2[beamends[i]] = true;
    }

    // beamstate == boolean for keeping track of whether or not a beam
    // is currently active.
    // bool beamstate = false;

    // Go back and link all partial beamed tuplets and non-beamed tuplets.
    // Need to consider this case: (12,24,12.) where the (12,24) are a power-of-two duration,
    // and the (12.) is also a power of two.  Maybe also merge single-tuplets into adjacent
    // tuplets and/or have a preference for grouping at the beat level rather than sub-beat.
    hum::HumNum groupdur;
    bool hasRest = false;
    for (int i = 0; i < (int)poweroftwo.size(); ++i) {
        if ((!beamstarts.empty()) && beamstarts2.at(i)) {
            // beamstate = true;
        }

        if (poweroftwo.at(i)) {
            // not a tuplet
            continue;
        }
        if (tupletgroups.at(i)) {
            // already in a tuplet group
            continue;
        }
        // At a tuplet which is not already in a tuplet group
        // search for how long the group should occur.
        int j = i + 1;
        int ending = (int)poweroftwo.size() - 1;
        groupdur = 0;
        while (j < (int)poweroftwo.size()) {
            if ((!hasRest) && (duritems[j]->isRest())) {
                hasRest = true;
            }
            if (poweroftwo[j]) {
                ending = j - 1;
                break;
            }
            if (tupletgroups.at(j)) {
                ending = j - 1;
                break;
            }
            groupdur = dursum[j] - dursum[i] + durationwithdots[j];
            if (groupdur.isPowerOfTwo()) {
                ending = j;
                break;
            }
            j++;
        }
        if (ending >= 0) {
            // create a new tuplet group (which will require a bracket).
            for (j = i; j <= ending; ++j) {
                tupletgroups.at(j) = tupletnum;
                // Only turn on a tuplet bracket if the tuplet is not inside
                // of a beam (may have to change if a tuplet bracket is
                // desired within a beam).
                if (!hasRest) {
                    tupletbracket[j] = true;
                }
            }
            tupletnum++;
            i = ending;
        }

        if ((!beamends2.empty()) && beamends2[i]) {
            // beamstate = false;
        }
    }

    checkForTupletMergesAndSplits(tupletgroups, duritems, durationwithdots, durforce);

    // tupletstartboolean == starting of a tuplet group
    // tupletendboolean == ending of a tuplet group
    std::vector<int> tupletstartboolean(tupletgroups.size(), 0);
    std::vector<int> tupletendboolean(tupletgroups.size(), 0);
    std::vector<bool> tstart(tupletgroups.size(), false);
    std::vector<bool> tend(tupletgroups.size(), false);
    for (int i = 0; i < (int)tupletgroups.size(); ++i) {
        if (!tupletgroups.at(i)) {
            continue;
        }
        if (!tstart.at(tupletgroups.at(i) - 1)) {
            tupletstartboolean[i] = tupletgroups.at(i);
            tstart.at(tupletgroups.at(i) - 1) = true;
        }
    }

    for (int i = (int)tupletgroups.size() - 1; i >= 0; i--) {
        if (!tupletgroups.at(i)) {
            continue;
        }
        if (!tend.at(tupletgroups.at(i) - 1)) {
            tupletendboolean.at(i) = tupletgroups.at(i);
            tend.at(tupletgroups.at(i) - 1) = true;
        }
    }

    std::vector<int> tuptop(tupletgroups.size(), -1);
    std::vector<int> tupbot(tupletgroups.size(), -1);
    for (int i = 0; i < (int)tupletgroups.size(); ++i) {
        if (!tupletgroups.at(i)) {
            continue;
        }
        hum::HumNum nextpowoftwo;
        if (dotlessdur[i] < 1) {
            nextpowoftwo = nextHigherPowerOfTwo(dotlessdur[i]);
        }
        else {
            nextpowoftwo = nextLowerPowerOfTwo((double)tuptop[i] / tupbot[i]);
        }

        if (dotlessdur[i].getNumerator() == 3) {
            hum::HumNum testval = dotlessdur[i].getDenominator();
            if (testval.isPowerOfTwo()) {
                // correction for duplets
                nextpowoftwo /= 2;
            }
        }
        hum::HumNum value = dotlessdur[i] / nextpowoftwo;
        tuptop[i] = value.getDenominator();
        tupbot[i] = value.getNumerator();

        // Reference tuplet breve do breve rather than whole.
        if ((dotlessdur[i].getNumerator() == 4) && (dotlessdur[i].getDenominator() == 3)) {
            tupbot[i] = 2;
        }
    }

    // adjust tupletgroups based on tuptop and tupbot changes
    int correction = 0;
    for (int i = 1; i < (int)tuptop.size(); ++i) {
        if ((tuptop[i] == 1) && (tupbot[i] == 1)) {
            continue;
        }
        if ((tuptop[i] == -1) && (tupbot[i] == -1)) {
            continue;
        }
        if ((tuptop[i - 1] == 1) && (tupbot[i - 1] == 1)) {
            continue;
        }
        if ((tuptop[i - 1] == -1) && (tupbot[i - 1] == -1)) {
            continue;
        }

        if ((tuptop[i] != tuptop[i - 1]) || (tupbot[i] != tupbot[i - 1])) {
            if (tupletgroups.at(i) == tupletgroups.at(i - 1)) {
                correction++;
                tupletstartboolean[i] = true;
                tupletendboolean[i - 1] = true;
            }
        }
        tupletgroups.at(i) += correction;
    }

    for (int i = 0; i < (int)tuptop.size(); ++i) {
        if (tuptop[i] < 0) {
            tuptop[i] = -tuptop[i];
        }
    }

    // tupletscale == 3 for three triplets, 6 for six sextuplets.
    // int xmin = 0;
    // int state = 0;
    // int value = 0;
    // int starti = -1;
    hum::HumNum vdur;
    hum::HumNum val2;
    std::vector<int> tupletscale(tupletstartboolean.size(), 1);

    for (int i = 0; i < (int)tupletstartboolean.size(); ++i) {
        hum::HumNum xx = groupdur / 4 / dotlessdur[i] / tuptop[i];
        tupletscale[i] = xx.getNumerator();
    }

    tgs.resize(layerdata.size());
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        tgs.at(i).token = layerdata[i];
        if (indexmapping2[i] < 0) {
            // this is a non-durational layer item or a non-tuplet note.
            tgs.at(i).duration = 0;
            tgs.at(i).durationnodots = 0;
            tgs.at(i).group = -1;
            tgs.at(i).bracket = -1;
            tgs.at(i).num = -1;
            tgs.at(i).numbase = -1;
            tgs.at(i).numscale = 1;
            tgs.at(i).beamstart = 0;
            tgs.at(i).beamend = 0;
            tgs.at(i).gbeamstart = gbeamstart.at(i);
            tgs.at(i).gbeamend = gbeamend.at(i);
            tgs.at(i).tupletstart = 0;
            tgs.at(i).tupletend = 0;
            tgs.at(i).force = false;
            tgs.at(i).priority = ' ';
        }
        else {
            // this is a tuplet note (with duration)
            tgs.at(i).duration = layerdata[i]->getDuration();
            tgs.at(i).durationnodots = layerdata[i]->getDurationNoDots();
            tgs.at(i).group = tupletgroups.at(indexmapping2.at(i));
            tgs.at(i).bracket = tupletbracket.at(indexmapping2.at(i));
            tgs.at(i).num = tuptop.at(indexmapping2.at(i));
            tgs.at(i).numbase = tupbot.at(indexmapping2.at(i));
            tgs.at(i).beamstart = beamstartboolean.at(indexmapping2.at(i));
            tgs.at(i).beamend = beamendboolean.at(indexmapping2.at(i));
            tgs.at(i).gbeamstart = gbeamstart.at(i);
            tgs.at(i).gbeamend = gbeamend.at(i);
            tgs.at(i).tupletstart = tupletstartboolean.at(indexmapping2.at(i));
            tgs.at(i).force = durforce.at(indexmapping2.at(i));
            tgs.at(i).tupletend = tupletendboolean.at(indexmapping2.at(i));
            tgs.at(i).numscale = 1; // initialize numscale and fill in later in assignTupletScalings()
        }
    }

    // Renumber tuplet groups in sequence (otherwise the mergeTupletsCuttingBeam()
    // function will delete the 1st group if it is not the first tuplet.
    int tcounter = 0;
    for (int i = 0; i < (int)tgs.size(); ++i) {
        if (tgs.at(i).tupletstart) {
            tgs.at(i).tupletstart = ++tcounter;
        }
        else if (tgs.at(i).tupletend) {
            tgs.at(i).tupletend = tcounter;
        }
    }

    mergeTupletsCuttingBeam(tgs);
    resolveTupletBeamTie(tgs);
    assignTupletScalings(tgs);

    storeTupletAndBeamInfoInTokens(tgs);
}

//////////////////////////////
//
// HumdrumInput::storeTupletAndBeamInfoInTokens --
//

void HumdrumInput::storeTupletAndBeamInfoInTokens(std::vector<humaux::HumdrumBeamAndTuplet> &tgs)
{
    for (int i = 0; i < (int)tgs.size(); ++i) {
        if (tgs[i].beamstart) {
            tgs[i].token->setValue("auto", "beamstart", tgs[i].beamstart);
        }
        if (tgs[i].beamend) {
            tgs[i].token->setValue("auto", "beamend", tgs[i].beamend);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::assignTupletScalings --
//

void HumdrumInput::assignTupletScalings(std::vector<humaux::HumdrumBeamAndTuplet> &tg)
{
    int maxgroup = 0;
    for (int i = 0; i < (int)tg.size(); ++i) {
        if (maxgroup < tg[i].group) {
            maxgroup = tg[i].group;
        }
    }
    if (maxgroup <= 0) {
        // no tuplets
        return;
    }

    // tggroups is a list of only durational items, removing things like clefs and barlines.
    std::vector<std::vector<humaux::HumdrumBeamAndTuplet *>> tggroups(maxgroup + 1);
    for (int i = 0; i < (int)tg.size(); ++i) {
        int group = tg[i].group;
        if (group <= 0) {
            continue;
        }
        tggroups.at(group).push_back(&tg[i]);
    }
    for (int i = 1; i < (int)tggroups.size(); ++i) {
        assignScalingToTupletGroup(tggroups[i]);
    }
}

//////////////////////////////
//
// HumdrumInput::assignScalingToTupletGroup --
//

void HumdrumInput::assignScalingToTupletGroup(std::vector<humaux::HumdrumBeamAndTuplet *> &tggroup)
{
    if (tggroup.empty()) {
        return;
    }

    // Set a specific number for the tuplet (which make sense).
    std::string num = tggroup[0]->token->getLayoutParameter("TUP", "num");
    if (!num.empty()) {
        int numvalue = stoi(num);
        if (numvalue > 0) {
            hum::HumNum scale = num;
            scale /= tggroup[0]->num;
            if (scale.isInteger() && (scale >= 1)) {
                for (int i = 0; i < (int)tggroup.size(); ++i) {
                    tggroup[i]->numscale = scale.getNumerator();
                }
                return;
            }
        }
    }

    // initialize all scalings to 1
    for (int i = 0; i < (int)tggroup.size(); ++i) {
        tggroup[i]->numscale = 1;
    }

    std::map<hum::HumNum, int> durcounts;
    for (int i = 0; i < (int)tggroup.size(); ++i) {
        durcounts[tggroup[i]->durationnodots]++;
    }

    // All durations are the same, so set the scale to the multiple of how
    // many of that duration are present.
    if (durcounts.size() == 1) {
        hum::HumNum scale = durcounts.begin()->second;
        scale /= tggroup[0]->num;
        if (scale.isInteger() && (scale > 1)) {
            for (int i = 0; i < (int)tggroup.size(); ++i) {
                tggroup[i]->numscale = scale.getNumerator();
            }
        }
        return;
    }

    if (durcounts.size() == 2) {
        auto it = durcounts.begin();
        int count1 = it->second;
        it++;
        int count2 = it->second;
        if (count1 == count2) {
            hum::HumNum scale = count1;
            scale /= tggroup[0]->num;
            if (scale.isInteger() && (scale > 1)) {
                for (int i = 0; i < (int)tggroup.size(); ++i) {
                    tggroup[i]->numscale = scale.getNumerator();
                }
            }
            return;
        }
    }

    /*
    // Use the most common duration for the tuplet scaling:
    // (this could be refined for dotted durations)
    hum::HumNum maxcountdur = 0;
    int maxcount = 0;
    for (auto it : durcounts) {
        if (it.second > maxcount) {
            maxcount = it.second;
            maxcountdur = it.first;
        }
    }
    */

    // Select the longest duration if there is a tie.
    hum::HumNum maxcountdur = 0;
    for (auto it : durcounts) {
        if (it.first > maxcountdur) {
            maxcountdur = it.first;
        }
    }

    hum::HumNum totaldur = 0;
    for (int i = 0; i < (int)tggroup.size(); ++i) {
        totaldur += tggroup[i]->duration;
    }

    hum::HumNum units = totaldur;
    units /= maxcountdur;

    if (units.isInteger() && (units > 1)) {
        hum::HumNum scale = units;
        scale /= tggroup[0]->num;
        if (scale.isInteger() && (scale > 1)) {
            for (int i = 0; i < (int)tggroup.size(); ++i) {
                tggroup[i]->numscale = scale.getNumerator();
            }
            return;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForTupletMergesAndSplits -- check to see if an automatically
//    assigned tuplet group should be merged or split based on layout commands
//    attached to tuplet notes.  Only the first note of a tuplet group will be checked.
//    For tuplet splitting, set the duration of the first group to shorter than the
//    automatic grouping, and then optionally set the duration of the secondary
//    subgroup(s).
//
// Example:
//    !LO:TUP:r=2
//
//  r=2 means that the tuplet starting on the next note should last for the duration
//  of a half note (r means "rhythm" or specifically "**recip".
//

void HumdrumInput::checkForTupletMergesAndSplits(std::vector<int> &tupletgroups, std::vector<hum::HTp> &duritems,
    std::vector<hum::HumNum> &durations, std::vector<bool> &durforce)
{

    int counter = -1;
    int lastgroup = 0;
    hum::HumNum sum;
    hum::HumNum targetsum;
    for (int i = 0; i < (int)tupletgroups.size(); ++i) {
        if (tupletgroups.at(i) == 0) {
            continue;
        }
        if (tupletgroups.at(i) == lastgroup) {
            continue;
        }
        std::string rparam = duritems.at(i)->getLayoutParameter("TUP", "r");
        if (rparam.empty()) {
            lastgroup = tupletgroups.at(i);
            continue;
        }

        targetsum = hum::Convert::recipToDuration(rparam);
        sum = 0;
        for (int j = i; j < (int)tupletgroups.size(); j++) {
            if (tupletgroups.at(j) == 0) {
                // do not allow tuplets outside on non-tuplet notes
                break;
            }
            sum += durations.at(j);
            if (sum <= targetsum) {
                tupletgroups.at(j) = counter;
                durforce.at(j) = true;
            }
            if (sum >= targetsum) {
                break;
            }
        }
        lastgroup = tupletgroups.at(i);
        counter--;
    }

    if (counter == -1) {
        // nothing was updated in tuplet groupings
        return;
    }

    counter = 0;
    lastgroup = 0;
    for (int i = 0; i < (int)tupletgroups.size(); ++i) {
        if (tupletgroups.at(i) == 0) {
            continue;
        }
        if (tupletgroups.at(i) != lastgroup) {
            lastgroup = tupletgroups.at(i);
            counter++;
            for (int j = i; j < (int)tupletgroups.size(); j++) {
                i = j;
                if (tupletgroups.at(j) == lastgroup) {
                    tupletgroups.at(j) = counter;
                }
                else {
                    i = j - 1;
                    break;
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForTupletForcedBreak --
//

bool HumdrumInput::checkForTupletForcedBreak(const std::vector<hum::HTp> &duritems, int index)
{
    if (index == 0) {
        return false;
    }
    if (index > (int)duritems.size()) {
        return false;
    }

    hum::HTp starttok = duritems[index];
    hum::HTp endtok = duritems[index - 1];
    int stopline = endtok->getLineIndex();
    int curline = starttok->getLineIndex();
    hum::HTp cur = starttok->getPreviousToken();
    while (cur && (curline > stopline)) {
        if (cur->isInterpretation() && (*cur == "*tupbreak")) {
            return true;
        }
        cur = cur->getPreviousToken();
        curline = cur->getLineIndex();
        if (cur == endtok) {
            break;
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::mergeTupletsCuttingBeam -- When a tuplet ends on a beamed note,
//     but it can be continued with another tuplet of the same type, then merge the
//     two tuplets.  These cases are caused by groupings needed at a higher level
//     than the beat according to the time signature.
//

void HumdrumInput::mergeTupletsCuttingBeam(std::vector<humaux::HumdrumBeamAndTuplet> &tg)
{

    // newtg is a list of only durational items, removing things like clefs and barlines.
    std::vector<humaux::HumdrumBeamAndTuplet *> newtg;
    for (int i = 0; i < (int)tg.size(); ++i) {
        if (tg.at(i).group >= 0) {
            newtg.push_back(&tg.at(i));
        }
    }

    std::vector<int> inbeam(newtg.size(), 0);
    for (int i = 0; i < (int)inbeam.size(); ++i) {
        if (newtg.at(i)->force) {
            inbeam.at(i) = 0;
        }
        else if (newtg.at(i)->beamstart) {
            inbeam.at(i) = newtg.at(i)->beamstart;
        }
        else if (newtg.at(i)->beamend) {
            inbeam.at(i) = 0;
        }
        else if (i > 0) {
            inbeam.at(i) = inbeam.at(i - 1);
        }
        else {
            inbeam.at(i) = 0;
        }
    }

    std::vector<int> scaleadj(newtg.size(), 1);

    for (int i = 0; i < (int)newtg.size(); ++i) {
        if (!(inbeam.at(i) && newtg.at(i)->tupletend)) {
            continue;
        }
        if (i >= (int)newtg.size() - 1) {
            continue;
        }
        if (!newtg.at(i + 1)->tupletstart) {
            continue;
        }
        if (newtg.at(i)->num != newtg.at(i + 1)->num) {
            continue;
        }
        if (newtg.at(i)->numbase != newtg.at(i + 1)->numbase) {
            continue;
        }
        // Need to merge adjacent tuplets (only can merge one pair at a time
        // properly with scaleadj at the moment).
        int target = newtg.at(i)->tupletend;
        for (int j = i; j >= 0; j--) {
            if (!newtg.at(j)->tupletstart) {
                scaleadj.at(j) = 2;
                continue;
            }
            if (target == newtg.at(j)->tupletstart) {
                scaleadj.at(j) = 2;
                break;
            }
            cerr << "SOMETHING STRANGE HAPPENED HERE" << endl;
        }
        target = newtg.at(i + 1)->tupletstart;
        scaleadj.at(i) = 2;
        scaleadj.at(i + 1) = 2;
        for (int j = i + 1; j < (int)newtg.size(); j++) {
            if (newtg.at(j)->group < 0) {
                continue;
            }
            if (!newtg.at(j)->tupletend) {
                scaleadj.at(j) = 2;
                continue;
            }
            if (target == newtg.at(j)->tupletend) {
                scaleadj.at(j) = 2;
                break;
            }
            cerr << "SOMETHING STRANGE HAPPENED HERE2" << endl;
        }

        newtg.at(i)->tupletend = 0;
        newtg.at(i + 1)->tupletstart = 0;
        for (int j = i + 2; j < (int)newtg.size(); j++) {
            if (newtg.at(j)->tupletstart) {
                newtg.at(j)->tupletstart--;
            }
            if (newtg.at(j)->tupletend) {
                newtg.at(j)->tupletend--;
            }
        }
    }

    // recalculate tuplet groups
    int currgroup = 0;
    for (int i = 0; i < (int)newtg.size(); ++i) {
        if (newtg[i]->tupletstart) {
            currgroup = newtg[i]->tupletstart;
        }
        newtg[i]->group = currgroup;
        if (newtg[i]->tupletend) {
            currgroup = 0;
        }
    }

    if (m_debug) {
        cerr << "INDEX\tBEAM\tTSTART\tTEND\tNUM\tNUMBASE\n";
        for (int i = 0; i < (int)newtg.size(); ++i) {
            cerr << "I " << i << ":\t" << inbeam.at(i) << "\t" << newtg.at(i)->tupletstart << "\t"
                 << newtg.at(i)->tupletend << "\t" << newtg.at(i)->num << "\t" << newtg.at(i)->numbase
                 << "\tSA=" << scaleadj.at(i) << endl;
        }
    }

    for (int i = 0; i < (int)newtg.size(); ++i) {
        if (newtg.at(i)->group < 0) {
            continue;
        }
        newtg.at(i)->numscale *= scaleadj.at(i);
    }
}

//////////////////////////////
//
// HumdrumInput::resolveTupletBeamTie -- When a tuplet and beam both start or
// end
//   on the same note, figure out which one should be first, last.
//

void HumdrumInput::resolveTupletBeamTie(std::vector<humaux::HumdrumBeamAndTuplet> &tg)
{
    for (int i = 0; i < (int)tg.size(); ++i) {
        if (tg.at(i).beamstart && tg.at(i).tupletstart) {
            resolveTupletBeamStartTie(tg, i);
        }
        else if (tg.at(i).beamend && tg.at(i).tupletend) {
            resolveTupletBeamEndTie(tg, i);
        }
        else {
            tg.at(i).priority = ' ';
        }
    }
}

//////////////////////////////
//
// HumdrumInput::resolveTupletBeamStartTie -- When a tuplet and a beam start
//    on the same note, determine which one should be opened first (the one
//    which is contained by the other will be opened last, and if there is a
//    tie,
//    the tuplet will be opened first.
//

void HumdrumInput::resolveTupletBeamStartTie(std::vector<humaux::HumdrumBeamAndTuplet> &tg, int index)
{

    // presumably tumnum and beamnum are non-zero...
    int tupnum = tg.at(index).tupletstart;
    int beamnum = tg.at(index).beamstart;
    for (int i = index; i < (int)tg.size(); ++i) {
        if (tg.at(i).beamend == beamnum) {
            // beam ends before tuplet does, so tuplet should be give priority.
            tg.at(index).priority = 'T'; // open tuplet first
            return;
        }
        if (tg.at(i).tupletend == tupnum) {
            // tuplet ends before beam does, so beam should be opened first.
            tg.at(index).priority = 'B'; // open beam first
            return;
        }
    }
    // strange problem
    tg.at(index).priority = ' ';
}

//////////////////////////////
//
// HumdrumInput::resolveTupletBeamEndTie -- When a tuplet and a beam end
//    on the same note, determine which one should be closed first (the one
//    which is contained by the other will be closed first, and if there is a
//    tie,
//    the tuplet will be closed last.
//

void HumdrumInput::resolveTupletBeamEndTie(std::vector<humaux::HumdrumBeamAndTuplet> &tg, int index)
{

    // presumably tumnum and beamnum are non-zero...
    int tupnum = tg.at(index).tupletend;
    int beamnum = tg.at(index).beamend;
    for (int i = index; i >= 0; i--) {
        if (tg.at(i).beamstart == beamnum) {
            // beam starts after tuplet does, so beam should be closed first.
            tg.at(index).priority = 'B'; // close beam first
            return;
        }
        if (tg.at(i).tupletstart == tupnum) {
            // tuplet starts after beam does, so tuplet should be closed first.
            tg.at(index).priority = 'T'; // close tuplet first
            return;
        }
    }
    // strange problem
    tg.at(index).priority = ' ';
}

//////////////////////////////
//
// HumdrumInput::nextHigherPowerOfTwo -- Use with values between 0 and 1.
//

hum::HumNum HumdrumInput::nextHigherPowerOfTwo(hum::HumNum x)
{
    double value = log(x.getFloat()) / log(2.0);
    value = -value;
    int denom = int(value);
    hum::HumNum rval = 1;
    rval /= (int)pow(2.0, denom);
    return rval;
}

//////////////////////////////
//
// HumdrumInput::nextLowerPowerOfTwo -- For integers above 1.
//

int HumdrumInput::nextLowerPowerOfTwo(int x)
{
    if (x < 1) {
        return 1;
    }
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    return x - (x >> 1);
}

//////////////////////////////
//
// HumdrumInput::getDotPowerOfTwo -- Checks up to 3 augmentation dots.
//

int HumdrumInput::getDotPowerOfTwo(hum::HumNum value)
{
    if (value.isPowerOfTwo()) {
        return 0;
    }
    // check for one dot
    hum::HumNum tval = value * 2 / 3;
    if (tval.isPowerOfTwo()) {
        return 1;
    }
    tval = value * 4 / 7;
    if (tval.isPowerOfTwo()) {
        return 2;
    }
    tval = value * 8 / 15;
    if (tval.isPowerOfTwo()) {
        return 3;
    }

    return -1;
}

//////////////////////////////
//
// HumdrumInput::removeFactorsOfTwo --
//

hum::HumNum HumdrumInput::removeFactorsOfTwo(hum::HumNum value, int &tcount, int &bcount)
{
    int top = value.getNumerator();
    int bot = value.getDenominator();
    tcount = 0;
    bcount = 0;
    if (top > 0) {
        while (top % 2 == 0) {
            top = top >> 1;
            tcount++;
        }
    }
    if (bot > 0) {
        while (bot % 2 == 0) {
            bot = bot >> 1;
            bcount++;
        }
    }
    hum::HumNum output(top, bot);
    return output;
}

//////////////////////////////
//
// HumdrumInput::handleStaffStateVariables -- Deal with simple boolean switches
//     that are turned on/off by interpretation tokens.
//
// Controls that this function deals with:
//    *artic       = show articulations on notes
//    *Xartic      = hide articulations on notes
//
//    *Xtuplet     = suppress beam and bracket tuplet numbers
//    *tuplet      = display beam and bracket tuplet numbers
//
//    *Xtremolo    = terminal *tremelo contraction
//    *tremolo     = merge possible beam groups into tremolos
//
//    *Xbeamtup    = suppress beam tuplet numbers
//    *beamtup     = display beam tuplet numbers
//
//    *Xbrackettup = suppress tuplet brackets
//    *brackettup  = display tuplet brackets
//
//    *Xcue        = notes back to regular size (operates at layer level rather than staff level).
//    *cue         = display notes in cue size (operates at layer level rather than staff level).
//
//    *kcancel     = display cancellation key signatures
//    *Xkcancel    = do not display cancellation key signatures (default)
//
//    *2\right     = place stems on right side of half notes when stem is down.
//    *2\left      = place stems on left side of half notes when stem is down.
//
//    *stem:       = automatic assignment of stems if there are no stems on the note already.
//       *stem:X   = no automatic assignment
//       *stem:x   = no stem
//       *stem:/   = stem up
//       *stem:\   = stem down
//
//    *head:       = notehead shape
//

void HumdrumInput::handleStaffStateVariables(hum::HTp token)
{
    int staffindex = m_currentstaff - 1;
    int layernum = m_currentlayer;
    std::string value = *token;
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    if (value == "*Xbeamtup") {
        ss[staffindex].suppress_tuplet_number = true;
    }
    else if (value == "*beamtup") {
        ss[staffindex].suppress_tuplet_number = false;
    }
    if (value == "*Xbrackettup") {
        ss[staffindex].suppress_tuplet_bracket = true;
    }
    else if (value == "*brackettup") {
        ss[staffindex].suppress_tuplet_bracket = false;
    }

    if ((value == "*Xartic") || (value.compare(0, 8, "*Xartic:") == 0)) {
        ss[staffindex].suppress_articulations = true;
    }
    else if ((value == "*artic") || (value.compare(0, 7, "*artic:") == 0)) {
        ss[staffindex].suppress_articulations = false;
    }

    if (value == "*Xtuplet") {
        ss[staffindex].suppress_tuplet_number = true;
    }
    else if (value.compare(0, 7, "*tuplet") == 0) {
        ss[staffindex].suppress_tuplet_number = false;
    }

    if (value == "*Xtremolo") {
        ss[staffindex].tremolo = false;
    }
    else if (value == "*tremolo") {
        ss[staffindex].tremolo = true;
        m_hasTremolo = true;
    }

    if (value == "*Xcue") {
        ss[staffindex].cue_size.at(layernum) = false;
    }
    else if (value == "*cue") {
        ss[staffindex].cue_size.at(layernum) = true;
    }

    else if (value.substr(0, 5) == "*stem") {
        storeStemInterpretation(value, staffindex, token->getSubtrack());
    }
    else if (value.substr(0, 6) == "*Xstem") {
        storeStemInterpretation(value, staffindex, token->getSubtrack());
    }

    else if (value.find("acclev") != std::string::npos) {
        storeAcclev(value, staffindex);
    }

    else if (value == "*2\\left") {
        ss[staffindex].righthalfstem = false;
    }
    else if (value == "*2\\right") {
        ss[staffindex].righthalfstem = true;
    }

    // Key cancellation option is currently global to all staves:
    if (value == "*Xkcancel") {
        m_show_cautionary_keysig = false;
    }
    else if (value == "*kcancel") {
        m_show_cautionary_keysig = true;
    }

    if (value.compare(0, 6, "*head:") == 0) {
        ss[staffindex].m_notehead.clear();
        for (int i = 6; i < (int)value.size(); ++i) {
            if (value[i] == ':') {
                // There may be a pitch parameter after the shape,
                // but this is ignored for now.
                break;
            }
            ss[staffindex].m_notehead += value[i];
        }
    }
    else if (value == "*Xhead") {
        ss[staffindex].m_notehead = "regular";
    }
}

//////////////////////////////
//
// HumdrumInput::storeStemInterpretation --
//

void HumdrumInput::storeStemInterpretation(const std::string &value, int staffindex, int layernumber)
{
    if (value.find("stem") == std::string::npos) {
        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if ((value.size() >= 7) && (value.compare(0, 6, "*stem:") == 0)) {
        std::string ending = value.substr(6);
        if (ending == "x") {
            ss[staffindex].stem_type.at(layernumber) = 'x';
        }
        else if (ending == "/") {
            // force notes to have stem up starting here
            ss[staffindex].stem_type.at(layernumber) = '/';
        }
        else if (ending == "\\") {
            // force notes to have stem down starting here
            ss[staffindex].stem_type.at(layernumber) = '\\';
        }
        else {
            ss[staffindex].stem_type.at(layernumber) = 'X';
        }
    }
    else if (value == "*stem") {
        // show stems (default behavior)
        if (layernumber == 0) {
            std::fill(ss[staffindex].stem_visible.begin(), ss[staffindex].stem_visible.end(), true);
        }
        else {
            ss[staffindex].stem_visible.at(layernumber) = true;
        }
    }
    else if (value == "*Xstem") {
        // hide stems
        if (layernumber == 0) {
            std::fill(ss[staffindex].stem_visible.begin(), ss[staffindex].stem_visible.end(), false);
        }
        else {
            ss[staffindex].stem_visible.at(layernumber) = false;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::storeAcclev -- Used for **mens accidental conversion to @accid+@edit or @accid.ges.
//

void HumdrumInput::storeAcclev(const std::string value, int staffindex)
{
    if (value.find("acclev") == std::string::npos) {
        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if ((value.size() > 8) && (value.substr(0, 8) == "*acclev:")) {
        std::string state = value.substr(8);
        if (!state.empty()) {
            if (isdigit(state[0])) {
                ss[staffindex].acclev = state[0] - '0';
            }
            else if (state == "YY") {
                ss[staffindex].acclev = 1;
            }
            else if (state == "Y") {
                ss[staffindex].acclev = 2;
            }
            else if (state == "yy") {
                ss[staffindex].acclev = 3;
            }
            else if (state == "y") {
                ss[staffindex].acclev = 4;
            }
        }
    }
    else if (value == "*acclev:") {
        ss[staffindex].acclev = 0;
    }
    else if (value == "*acclev") {
        ss[staffindex].acclev = 0;
    }
    else if (value == "*Xacclev") {
        ss[staffindex].acclev = 0;
    }
}

//////////////////////////////
//
// HumdrumInput::handleStaffDynamStateVariables -- Deal with simple boolean switches
//     that are turned on/off by interpretation tokens in **dynam spines.
//
// NB: need to set to part level rather than staff level?
//
// Controls that this function deals with:
//    *above   = Force all dynamics above staff.
//    *above:2 = Force all dynamics above staff below top one
//    *below   = Force all dynamics below the staff.
//    *below:2 = Force all dynamics below staff below top one
//    *center  = Force all dynamics to be centered between this staff and the one below.
//

void HumdrumInput::handleStaffDynamStateVariables(hum::HTp token)
{
    int staffindex = m_currentstaff - 1;
    std::string value = *token;
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    hum::HTp tok = token->getNextFieldToken();
    while ((tok != NULL) && (!tok->isKernLike())) {
        if (!tok->isDataType("**dynam")) {
            tok = tok->getNextFieldToken();
            continue;
        }
        if (*tok == "*above") {
            ss[staffindex].m_dynampos = +1;
            ss[staffindex].m_dynamstaffadj = 0;
        }
        else if (*tok == "*above:2") {
            ss[staffindex].m_dynampos = +1;
            ss[staffindex].m_dynamstaffadj = -1;
        }
        else if (*tok == "*below:2") {
            ss[staffindex].m_dynampos = -1;
            ss[staffindex].m_dynamstaffadj = -1;
        }
        else if (*tok == "*below") {
            ss[staffindex].m_dynampos = -1;
            ss[staffindex].m_dynamstaffadj = 0;
        }
        else if (*tok == "*center") {
            ss[staffindex].m_dynampos = 0;
            ss[staffindex].m_dynamposdefined = true;
            ss[staffindex].m_dynamstaffadj = 0;
        }
        else if (*tok == "*center:2") {
            // for centering on organ staff between pedal
            // and bottom of grand staff.
            ss[staffindex].m_dynampos = 0;
            ss[staffindex].m_dynamstaffadj = -1;
            ss[staffindex].m_dynamposdefined = true;
        }
        tok = tok->getNextFieldToken();
    }
}

//////////////////////////////
//
// HumdrumInput::handleOttavaMark --  *8va turns on and *X8va tuns off.
//    Generalized latter to 15ma, 8va basso, 15ma basso.  IF the *X8va is
//    (incorrectly) placed at the start of the next measure before a note,
//    then this algorithm may not work (need to keep track of the last note
//    in the previous measure among all layers).
//

void HumdrumInput::handleOttavaMark(hum::HTp token, Note *note)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_currentstaff - 1;

    if (*token == "*8va") {
        // turn on ottava
        ss[staffindex].ottavameasure = m_measure;
        ss[staffindex].ottavanotestart = NULL;
        ss[staffindex].ottavanoteend = NULL;
        ss[staffindex].ottavaendtimestamp = token->getDurationFromStart();
        // When a new note is read, check if ottavameasure
        // is non-null, and if so, store the new note in ottavanotestart.
    }
    else if (*token == "*8ba") {
        // turn on ottava down
        ss[staffindex].ottavadownmeasure = m_measure;
        ss[staffindex].ottavadownnotestart = NULL;
        ss[staffindex].ottavadownnoteend = NULL;
        ss[staffindex].ottavadownendtimestamp = token->getDurationFromStart();
        // When a new note is read, check if ottavadownmeasure
        // is non-null, and if so, store the new note in ottavadownnotestart.
    }
    else if (*token == "*15ba") {
        // turn on two ottava down
        ss[staffindex].ottava2downmeasure = m_measure;
        ss[staffindex].ottava2downnotestart = NULL;
        ss[staffindex].ottava2downnoteend = NULL;
        ss[staffindex].ottava2downendtimestamp = token->getDurationFromStart();
        // When a new note is read, check if ottava2downmeasure
        // is non-null, and if so, store the new note in ottava2downnotestart.
    }
    else if (*token == "*15ma") {
        // turn on ottava
        ss[staffindex].ottava2measure = m_measure;
        ss[staffindex].ottava2notestart = NULL;
        ss[staffindex].ottava2noteend = NULL;
        ss[staffindex].ottava2endtimestamp = token->getDurationFromStart();
        // When a new note is read, check if ottava2measure
        // is non-null, and if so, store the new note in ottava2notestart.
    }
    else if (*token == "*X8va") {
        // turn off ottava
        if ((ss[staffindex].ottavameasure != NULL) && (ss[staffindex].ottavanotestart != NULL)
            && (ss[staffindex].ottavanoteend != NULL)) {
            Octave *octave = new Octave();
            ss[staffindex].ottavameasure->AddChild(octave);
            setStaff(octave, staffindex + 1);
            octave->SetDis(OCTAVE_DIS_8);
            octave->SetStartid("#" + ss[staffindex].ottavanotestart->GetID());
            std::string endid = getEndIdForOttava(token);
            if (endid != "") {
                octave->SetEndid("#" + endid);
            }
            else {
                octave->SetEndid("#" + ss[staffindex].ottavanoteend->GetID());
            }
            octave->SetDisPlace(STAFFREL_basic_above);
        }
        ss[staffindex].ottavanotestart = NULL;
        ss[staffindex].ottavanoteend = NULL;
        ss[staffindex].ottavameasure = NULL;
        ss[staffindex].ottavaendtimestamp = 0;
    }
    else if (*token == "*X8ba") {
        // turn off ottava down
        if ((ss[staffindex].ottavadownmeasure != NULL) && (ss[staffindex].ottavadownnotestart != NULL)
            && (ss[staffindex].ottavadownnoteend != NULL)) {
            Octave *octave = new Octave();
            ss[staffindex].ottavadownmeasure->AddChild(octave);
            setStaff(octave, staffindex + 1);
            octave->SetDis(OCTAVE_DIS_8);
            octave->SetStartid("#" + ss[staffindex].ottavadownnotestart->GetID());
            std::string endid = getEndIdForOttava(token);
            if (endid != "") {
                octave->SetEndid("#" + endid);
            }
            else {
                octave->SetEndid("#" + ss[staffindex].ottavadownnoteend->GetID());
            }
            octave->SetDisPlace(STAFFREL_basic_below);
        }
        ss[staffindex].ottavadownnotestart = NULL;
        ss[staffindex].ottavadownnoteend = NULL;
        ss[staffindex].ottavadownmeasure = NULL;
        ss[staffindex].ottavadownendtimestamp = 0;
    }
    else if (*token == "*X15ma") {
        // turn off ottava2 up
        if ((ss[staffindex].ottava2measure != NULL) && (ss[staffindex].ottava2notestart != NULL)
            && (ss[staffindex].ottava2noteend != NULL)) {
            Octave *octave = new Octave();
            ss[staffindex].ottava2measure->AddChild(octave);
            setStaff(octave, staffindex + 1);
            octave->SetDis(OCTAVE_DIS_15);
            octave->SetStartid("#" + ss[staffindex].ottava2notestart->GetID());
            octave->SetEndid("#" + ss[staffindex].ottava2noteend->GetID());
            std::string endid = getEndIdForOttava(token);
            if (endid != "") {
                octave->SetEndid("#" + endid);
            }
            else {
                octave->SetEndid("#" + ss[staffindex].ottava2noteend->GetID());
            }
            octave->SetDisPlace(STAFFREL_basic_above);
        }
        ss[staffindex].ottava2notestart = NULL;
        ss[staffindex].ottava2noteend = NULL;
        ss[staffindex].ottava2measure = NULL;
        ss[staffindex].ottava2endtimestamp = 0;
    }
    else if (*token == "*X15ba") {
        // turn off ottava2 down
        if ((ss[staffindex].ottava2downmeasure != NULL) && (ss[staffindex].ottava2downnotestart != NULL)
            && (ss[staffindex].ottava2downnoteend != NULL)) {
            Octave *octave = new Octave();
            ss[staffindex].ottava2downmeasure->AddChild(octave);
            setStaff(octave, staffindex + 1);
            octave->SetDis(OCTAVE_DIS_15);
            octave->SetStartid("#" + ss[staffindex].ottava2downnotestart->GetID());
            std::string endid = getEndIdForOttava(token);
            if (endid != "") {
                octave->SetEndid("#" + endid);
            }
            else {
                octave->SetEndid("#" + ss[staffindex].ottava2downnoteend->GetID());
            }
            octave->SetDisPlace(STAFFREL_basic_below);
        }
        ss[staffindex].ottava2downnotestart = NULL;
        ss[staffindex].ottava2downnoteend = NULL;
        ss[staffindex].ottava2downmeasure = NULL;
        ss[staffindex].ottava2downendtimestamp = 0;
    }
}

//////////////////////////////
//
// HumdrumInput::getEndIdForOttava -- Find the last note before an ottava
//     line end across multiple layers.  If there is a tie between the
//     durations of the notes in different layers, choose the note from
//     a lower layer.  Do this by searching backwards in the token's strand
//     to find the first data token, then search accross all subspines
//     for the latest note.  It is assumed that the ottava marking is
//     placed in the position of the first layer (this is similar to
//     the assumption for clef changes).
//

std::string HumdrumInput::getEndIdForOttava(hum::HTp token)
{
    hum::HTp tok = token->getPreviousToken();
    while (tok && !tok->isData()) {
        tok = tok->getPreviousToken();
    }
    if (!tok) {
        // couldn't find a previous data line
        return "";
    }
    int track = tok->getTrack();
    int ttrack = track;
    std::vector<hum::HTp> notes;
    std::vector<hum::HumNum> timestamps;
    // for now counting rests as notes.
    while (ttrack == track) {
        hum::HTp xtok = tok;
        if (xtok->isNull()) {
            xtok = xtok->resolveNull();
        }
        if (!xtok) {
            tok = tok->getNextFieldToken();
            if (!tok) {
                break;
            }
            ttrack = tok->getTrack();
            continue;
        }
        notes.push_back(xtok);
        hum::HumNum timestamp = xtok->getDurationFromStart();
        timestamps.push_back(timestamp);
        tok = tok->getNextFieldToken();
        if (!tok) {
            break;
        }
        ttrack = tok->getTrack();
    }

    if (notes.empty()) {
        return "";
    }

    int bestindex = 0;
    for (int i = 1; i < (int)notes.size(); ++i) {
        if (timestamps[i] > timestamps[bestindex]) {
            bestindex = i;
        }
    }

    hum::HTp target = notes[bestindex];
    if (!target) {
        return "";
    }

    std::string prefix = "note";
    if (target->isRest()) {
        if (target->find("yy") != std::string::npos) {
            prefix = "space";
        }
        else {
            prefix = "rest";
        }
    }
    else if (target->isChord()) {
        prefix = "chord";
    }

    return getLocationId(prefix, target);
}

//////////////////////////////
//
// HumdrumInput::handleCustos --
//

void HumdrumInput::handleCustos(
    std::vector<std::string> &elements, std::vector<void *> &pointers, std::vector<hum::HTp> tokens, int index)
{
    hum::HTp token = tokens[index];
    hum::HumRegex hre;
    if (!hre.search(token, "^\\*(X*)custos(.*)")) {
        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_currentstaff - 1;

    std::string exes = hre.getMatch(1);
    if (exes == "X") {
        ss[staffindex].auto_custos = false;
        return;
    }
    if (exes == "XX") {
        ss[staffindex].suppress_manual_custos = true;
        return;
    }
    std::string ending = hre.getMatch(2);
    if (ending.empty()) {
        ss[staffindex].auto_custos = false;
        return;
    }

    if (ss[staffindex].suppress_manual_custos) {
        // Do not print any explicit custodes.
        return;
    }

    // add a manual custodes
    hre.search(ending, ":?([^:]*)(.*)");
    std::string kpitch = hre.getMatch(1);
    std::string parameters = hre.getMatch(2);

    if (kpitch.empty()) {
        // suppressing a custos here (deal with
        // this in the auto custos code elsewhere).
    }
    if ((kpitch == "x") || (kpitch == "X")) {
        // alises for suppressing an automatic custos
    }

    if (!hre.search(kpitch, "^[A-Ga-g]+[#n-]*$")) {
        // invalid manual custos (requires **kern pitch)
        return;
    }

    int base40 = hum::Convert::kernToBase40(kpitch);
    int oct = base40 / 40;
    // int acc = hum::Convert::base40ToAccidental(base40);
    int base7chroma = hum::Convert::base40ToDiatonic(base40) % 7;
    Custos *custos = new Custos();

    custos->SetOct(oct);
    switch (base7chroma) {
        case 0: custos->SetPname(PITCHNAME_c); break;
        case 1: custos->SetPname(PITCHNAME_d); break;
        case 2: custos->SetPname(PITCHNAME_e); break;
        case 3: custos->SetPname(PITCHNAME_f); break;
        case 4: custos->SetPname(PITCHNAME_g); break;
        case 5: custos->SetPname(PITCHNAME_a); break;
        case 6: custos->SetPname(PITCHNAME_b); break;
    }

    // switch (acc) {
    //    case +3: custos->SetAccid(ACCIDENTAL_WRITTEN_xs); break;
    //    case +2: custos->SetAccid(ACCIDENTAL_WRITTEN_x); break;
    //    case +1: custos->SetAccid(ACCIDENTAL_WRITTEN_s); break;
    //    case 0:
    //        if (kpitch.find("n") != std::string::npos) {
    //                custos->SetAccid(ACCIDENTAL_WRITTEN_n);
    //        }
    //        break;
    //    case -1: custos->SetAccid(ACCIDENTAL_WRITTEN_f); break;
    //    case -2: custos->SetAccid(ACCIDENTAL_WRITTEN_ff); break;
    //    case -3: custos->SetAccid(ACCIDENTAL_WRITTEN_tf); break;
    //}
    setLocationId(custos, token);
    appendElement(elements, pointers, custos);

    if (hre.search(parameters, "color=['\"]?([^'\":]+)['\":]?")) {
        std::string color = hre.getMatch(1);
        custos->SetColor(color);
    }
}

//////////////////////////////
//
// HumdrumInput::handlePedalMark --  *ped turns on and *Xped tuns off.
//    IF the *X8va is (incorrectly) placed at the start of the next measure
//    before a note, then this algorithm may not work (need to keep track
//    of the last note in the previous measure among all layers).
//

void HumdrumInput::handlePedalMark(hum::HTp token)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    int staffindex = m_currentstaff - 1;

    hum::HumNum durtobar = token->getDurationToBarline();
    hum::HumNum barbuffer(1, 4);

    if (*token == "*ped") {
        // turn on pedal
        Pedal *pedal = new Pedal();
        setLocationId(pedal, token);
        addChildMeasureOrSection(pedal);
        hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
        if (durtobar == 0) {
            tstamp -= barbuffer;
            appendTypeTag(pedal, "endbar-25");
        }
        hum::HTp attachment = getNextNonNullDataOrMeasureToken(token);
        setAttachmentType(pedal, attachment);
        pedal->SetDir(pedalLog_DIR_down);
        assignVerticalGroup(pedal, token);
        setStaff(pedal, m_currentstaff);
        if (ss[staffindex].pedal) {
            // already on, so turn off first
            pedal->SetDir(pedalLog_DIR_bounce);
            pedal->SetForm(PEDALSTYLE_altpedstar);
        }
        ss[staffindex].pedal = true;
    }
    else if (*token == "*Xped") {
        Pedal *pedal = new Pedal();
        setLocationId(pedal, token);
        addChildMeasureOrSection(pedal);
        hum::HumNum tstamp = getMeasureTstamp(token, staffindex, hum::HumNum(1, 1));
        if (durtobar == 0) {
            tstamp -= barbuffer;
            appendTypeTag(pedal, "endbar-25");
        }
        hum::HTp attachment = getNextNonNullDataOrMeasureToken(token);
        setAttachmentType(pedal, attachment);
        pedal->SetDir(pedalLog_DIR_up);
        assignVerticalGroup(pedal, token);
        setStaff(pedal, m_currentstaff);
        ss[staffindex].pedal = false;
    }
}

//////////////////////////////
//
// HumdrumInput::getNextNonNullDataOrMeasureToken -- Designed for
//    finding an attachment for Pedal marks.
//

hum::HTp HumdrumInput::getNextNonNullDataOrMeasureToken(hum::HTp tok)
{
    int track = tok->getTrack();

    hum::HTp current = tok->getNextToken();
    while (current) {
        if (current->isInterpretation()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isCommentLocal()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isNull()) {
            // Search for note in staff above.
            hum::HTp rcurrent = current->getNextFieldToken();
            while (rcurrent) {
                int rtrack = rcurrent->getTrack();
                if (abs(rtrack - track) > 1) {
                    break;
                }
                if (!rcurrent->isKern()) {
                    rcurrent = rcurrent->getNextFieldToken();
                    continue;
                }
                if (rcurrent->isNull()) {
                    rcurrent = rcurrent->getNextFieldToken();
                    continue;
                }
                if (rcurrent->isData()) {
                    return rcurrent;
                }
                rcurrent = rcurrent->getNextFieldToken();
            }

            current = current->getNextToken();
            continue;
        }
        if (current->isBarline()) {
            return current;
        }
        if (current->isData()) {
            return current;
        }
        current = current->getNextToken();
    }
    // Some sort of error and current is NULL pointer.
    return tok;
}

//////////////////////////////
//
// HumdrumInput::assignVerticalGroup -- Check for a layout parameter that sets the vertical
//    group for the pedal mark.  Use "!LO:PED:vg=none" to explicitly prevent a value
//    to be stored.  Use "!LO:PED:vg=default" for using the default group (which is currently
//    set to 200). "!LO:PED:vg=0" is equivalent to "vg=default", and using a negative integer
//    (or any other string) is equivalent to "vg=none".  In the future, there will be a
//    tandem interpretation (probably called *vgp, for vertical group pedal) that will control
//    the default vertical groupping (currenly hard-wired to the default (200) group if
//    a layout parameter is not given to alter it).
//

void HumdrumInput::assignVerticalGroup(Pedal *pedal, hum::HTp token)
{
    std::string vg = token->getLayoutParameter("PED", "vg");
    if (vg.empty()) {
        // use the default group
        pedal->SetVgrp(VGRP_PEDAL_DEFAULT);
    }
    else if (std::isdigit(vg[0])) {
        int value = stoi(vg);
        if (value < 0) {
            // do not assign a vertical group
        }
        else if (value == 0) {
            // use the default group
            pedal->SetVgrp(VGRP_PEDAL_DEFAULT);
        }
        else {
            // use the specified group
            pedal->SetVgrp(value);
        }
    }
    else if (vg == "default") {
        // use the default group
        pedal->SetVgrp(VGRP_PEDAL_DEFAULT);
    }
    else {
        // do not store a vertical group parameter for this pedal marking
    }
}

//////////////////////////////
//
// HumdrumInput::getPreviousDataToken --
//

hum::HTp HumdrumInput::getPreviousDataToken(hum::HTp token)
{
    if (token == NULL) {
        return NULL;
    }
    hum::HTp current = token;
    current = current->getPreviousToken();
    while (current) {
        if (current == NULL) {
            break;
        }
        if (current->isData()) {
            break;
        }
        current = current->getPreviousToken();
    }
    return current;
}

//////////////////////////////
//
// HumdrumInput::getChordNoteCount -- Count the number of notes in a chord,
//    removing invalid null chord notes and rests.
//

int HumdrumInput::getChordNoteCount(hum::HTp token)
{
    int scount = token->getSubtokenCount();
    bool isnote = false;
    int count = 0;
    std::string tstring;
    for (int i = 0; i < scount; ++i) {
        isnote = false;
        tstring = token->getSubtoken(i);
        if (tstring == "") {
            continue;
        }
        for (int k = 0; k < (int)tstring.size(); ++k) {
            if ((tstring[k] >= 'a') && (tstring[k] <= 'g')) {
                isnote = true;
                break;
            }
            else if ((tstring[k] >= 'A') && (tstring[k] <= 'G')) {
                isnote = true;
                break;
            }
        }
        if (isnote) {
            count++;
        }
    }
    return count;
}

//////////////////////////////
//
// HumdrumInput::convertChord --
//

void HumdrumInput::convertChord(Chord *chord, hum::HTp token, int staffindex)
{
    int scount = token->getSubtokenCount();
    int staffadj = getStaffAdjustment(token);
    if (staffadj != 0) {
        int staffnum = staffindex + 1 + staffadj;
        setStaff(chord, staffnum);
    }
    int k;
    bool isnote = false;
    bool isrest = false;
    bool isrecip = false;
    bool allinvis = true;

    std::vector<std::string> tstrings = token->getSubtokens();
    std::vector<Note *> notes;

    for (int i = 0; i < (int)tstrings.size(); ++i) {
        if (tstrings[i].find("yy") == std::string::npos) {
            allinvis = false;
            break;
        }
    }

    for (int j = 0; j < scount; ++j) {
        isnote = false;
        isrest = false;
        isrecip = false;
        std::string tstring = tstrings[j];
        if (tstring == "") {
            continue;
        }
        for (k = 0; k < (int)tstring.size(); ++k) {
            if (tstring[k] == 'r') {
                isrest = true;
            }
            else if ((tstring[k] >= 'a') && (tstring[k] <= 'g')) {
                isnote = true;
            }
            else if ((tstring[k] >= 'A') && (tstring[k] <= 'G')) {
                isnote = true;
            }
            else if ((tstring[k] >= '0') && (tstring[k] <= '9')) {
                isrecip = true;
            }
        }
        if (!(isnote || isrest || isrecip)) {
            continue;
        }

        if (isrest) {
            // <rest> not allowed in <chord>
            // (but chords are allowed in <rest>s somehow...)
            // continue;
            // Now a rest in a chord is a non-sounding notated notes
            // usually related to a fingered harmonic.
        }

        if (isrecip && !isnote) {
            // <space> not allowed in <chord>
            // (but chords are allowed in <space>es somehow...)
            continue;
        }

        // Suppress conversion of invisible notes.  This is becuase
        // verovio keeps the the stem visible for the invisible notes
        // in a chord.  Maybe allow when stems portions are also
        // invisible.
        if ((!allinvis) && (tstrings[j].find("yy") != std::string::npos)) {
            continue;
        }

        Note *note = new Note();
        setLocationId(note, token, j);
        appendElement(chord, note);
        convertNote(note, token, staffadj, staffindex, j);
        notes.push_back(note);
    }

    if (allinvis) {
        chord->SetVisible(BOOLEAN_false);
    }

    // grace notes need to be done before rhythm since default
    // duration is set to an eighth note.
    if (token->find("qq") != std::string::npos) {
        chord->SetGrace(GRACE_acc);
        // set the visual duration to an eighth note if there
        // is no rhythm specified (will be overwritten later
        // if there is a rhythm).
        chord->SetDur(DURATION_8);
    }
    else if (token->find("q") != std::string::npos) {
        chord->SetGrace(GRACE_unacc);
        // set the visual duration to an eighth note if there
        // is no rhythm specified (will be overwritten later
        // if there is a rhythm).
        chord->SetDur(DURATION_8);
    }

    hum::HumNum dur;
    if (m_hasTremolo && token->getValueBool("auto", "tremolo")) {
        hum::HumdrumToken newtok(token->getValue("auto", "recip"));
        dur = convertRhythm(chord, &newtok, 0);
    }
    else if (m_hasTremolo && token->getValueBool("auto", "tremolo2")) {
        hum::HumdrumToken newtok(token->getValue("auto", "recip"));
        dur = convertRhythm(chord, &newtok, 0);
    }
    else if (m_hasTremolo && token->getValueBool("auto", "tremoloAux")) {
        hum::HumdrumToken newtok(token->getValue("auto", "recip"));
        dur = convertRhythm(chord, &newtok, 0);
    }
    else {
        dur = convertRhythm(chord, token);
    }

    if (m_setrightstem) {
        m_setrightstem = false;
        chord->SetStemPos(STEMPOSITION_right);
    }

    int stemslashes = 0;
    if (m_signifiers.tremolo) {
        stemslashes = (int)std::count(token->begin(), token->end(), m_signifiers.tremolo);
    }
    if (stemslashes) {
        chord->SetStemMod(chord->AttStems::StrToStemmodifier(std::to_string(stemslashes) + "slash"));
    }

    // Stem direction of the chord.  If both up and down, then show up.
    int crossdir = token->getValueInt("auto", "stem.dir");
    if (crossdir == 1) {
        chord->SetStemDir(STEMDIRECTION_up);
    }
    else if (crossdir == -1) {
        chord->SetStemDir(STEMDIRECTION_down);
    }
    // Overwrite cross-stem direction if there is an explicit stem direction.
    if (token->find("/") != std::string::npos) {
        chord->SetStemDir(STEMDIRECTION_up);
        appendTypeTag(chord, "placed");
    }
    else if (token->find("\\") != std::string::npos) {
        chord->SetStemDir(STEMDIRECTION_down);
        appendTypeTag(chord, "placed");
    }

    adjustChordNoteDurations(chord, notes, tstrings);

    checkForAutoStem(chord, token);

    token->setValue("MEI", "xml:id", chord->GetID());
    int index = (int)m_measures.size() - 1;
    token->setValue("MEI", "measureIndex", index);

    // Add beam break information:
    int breaksec = token->getValueInt("", "auto", "breaksec");
    if (breaksec) {
        chord->SetBreaksec(breaksec);
    }

    convertVerses(chord, token);
}

//////////////////////////////
//
// HumdrumInput::adjustChordNoteDurations -- If the notes in a chord do not have
//    all of the same duration, set the duration of the notes which do not match
//    the chord's duration.  The chords duration is the duration of the first
//    note in the chord.  If a note does not have a duration, then it takes
//    the duration of the previous note (the first note requires a duration, or
//    it will be assigned a duration of a quarter note.
//

void HumdrumInput::adjustChordNoteDurations(
    Chord *chord, std::vector<Note *> &notes, std::vector<std::string> &tstrings)
{
    if (notes.size() != tstrings.size()) {
        return;
    }

    std::vector<hum::HumNum> durations(tstrings.size(), 0);
    hum::HumNum value;
    for (int i = 0; i < (int)tstrings.size(); ++i) {
        value = hum::Convert::recipToDuration(tstrings.at(i));
        if (value == 0) {
            if (i == 0) {
                value = 1;
            }
            else {
                value = durations.at(i - 1);
            }
        }
        durations.at(i) = value;
    }
    bool same = true;
    for (int i = 1; i < (int)durations.size(); ++i) {
        if (durations[0] != durations[i]) {
            same = false;
            break;
        }
    }
    if (same) {
        return;
    }

    int dots = chord->GetDots();
    int meidur = chord->GetDur();
    // meidur is a power of two, where 2 = whole note, 1 = breve, 0 = long, -1 = maxima
    // 3 = half note, 4 = quarter, 5 = eighth, etc.
    hum::HumNum hdur = 1;
    int powtwo = meidur - 2;
    if (powtwo > 0) {
        hdur /= 1 << powtwo;
    }
    else if (powtwo < 0) {
        hdur = 1 << -powtwo;
    }

    for (int i = 1; i < (int)durations.size(); ++i) {
        if (durations[0] != durations[i]) {
            hum::HumNum factor = durations[i] / durations[0];
            adjustChordNoteDuration(notes.at(i), hdur, meidur, dots, durations[0], tstrings[i], factor);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::adjustChordNoteDuration --
//
// dots == -1 means no @dots parameter.
//

void HumdrumInput::adjustChordNoteDuration(Note *note, hum::HumNum hdur, int meidur, int dots, hum::HumNum chorddur,
    const std::string &tstring, hum::HumNum factor)
{
    if (factor.isPowerOfTwo()) {
        // Handle simple case where dots are the same:
        int adjustment = (int)log2(factor.getFloat());
        setNoteMeiDur(note, meidur - adjustment);
        return;
    }

    // There is a difference in dot counts that also needs to be taken into account.
    int ndots = 0;
    for (int i = 0; i < (int)tstring.size(); ++i) {
        if (tstring[i] == '.') {
            ndots++;
        }
    }

    int dotdiff;
    if (dots < 0) {
        dotdiff = ndots;
    }
    else {
        dotdiff = ndots - dots;
    }
    if (dotdiff == 0) {
        // something strange happened
        return;
    }

    // check if the @dur of the note needs to be set
    hum::HumNum nodots = hum::Convert::recipToDurationNoDots(tstring);

    // converting hdur to whole-note units for comparison
    if ((nodots) != hdur * 4) {
        // different @dur, so set for note
        setRhythmFromDuration(note, nodots);
    }

    note->SetDots(ndots);
}

//////////////////////////////
//
// HumdrumInput::setNoteMeiDur -- Set the @dur attribute of a note.
//

void HumdrumInput::setNoteMeiDur(Note *note, int meidur)
{
    switch (meidur) {
        case -1: note->SetDur(DURATION_maxima); break;
        case 0: note->SetDur(DURATION_long); break;
        case 1: note->SetDur(DURATION_breve); break;
        case 2: note->SetDur(DURATION_1); break;
        case 3: note->SetDur(DURATION_2); break;
        case 4: note->SetDur(DURATION_4); break;
        case 5: note->SetDur(DURATION_8); break;
        case 6: note->SetDur(DURATION_16); break;
        case 7: note->SetDur(DURATION_32); break;
        case 8: note->SetDur(DURATION_64); break;
        case 9: note->SetDur(DURATION_128); break;
        case 10: note->SetDur(DURATION_256); break;
        case 11: note->SetDur(DURATION_512); break;
        case 12: note->SetDur(DURATION_1024); break;
        default: cerr << "UNKNOWN MEI DUR: " << meidur << endl;
    }
}

//////////////////////////////
//
// HumdrumInput::getTimingInformation -- Calculate the start time and duration
//     of each event so that partial layers can be filled in with <space>
//     elements if necessary.

void HumdrumInput::getTimingInformation(std::vector<hum::HumNum> &prespace, std::vector<hum::HTp> &layerdata,
    hum::HumNum layerstarttime, hum::HumNum layerendtime)
{
    prespace.resize(layerdata.size(), 0);
    if (m_mens) {
        // don't check mensural music durations
        return;
    }

    std::vector<int> dataindex;
    dataindex.reserve(layerdata.size());
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        if (layerdata.at(i)->isData()) {
            dataindex.push_back(i);
        }
    }

    std::vector<hum::HumNum> startdur(dataindex.size(), 0);
    std::vector<hum::HumNum> duration(dataindex.size(), 0);

    hum::HumNum correction = 0;
    for (int i = 0; i < (int)dataindex.size(); ++i) {
        int ii = dataindex.at(i);

        startdur.at(i) = layerdata.at(ii)->getDurationFromStart();
        if (!layerdata.at(ii)->isData()) {
            duration.at(i) = 0;
        }
        else if (layerdata.at(ii)->isNull()) {
            duration.at(i) = 0;
        }
        else {
            duration.at(i) = layerdata.at(ii)->getDuration();
        }
    }

    if (dataindex.size() > 0) {
        prespace.at(dataindex.at(0)) = startdur.at(0) - layerstarttime;
    }
    for (int i = 1; i < (int)dataindex.size(); ++i) {
        int ii = dataindex.at(i);
        prespace.at(ii) = startdur.at(i) - startdur.at(i - 1) - duration.at(i - 1);
        prespace.at(ii) -= m_duradj[layerdata[ii]->getLineIndex()];
        if (prespace.at(ii) < 0) {
            correction += prespace.at(ii);
            prespace.at(ii) = 0;
        }
        else if (prespace.at(ii) > 0) {
            prespace.at(ii) += correction;
            if (*layerdata.at(ii) != "*") {
                correction = 0;
            }
        }
    }

    bool clearDur = false;
    for (int i = 0; i < (int)dataindex.size(); ++i) {
        int ii = dataindex[i];
        if (layerdata[ii]->isData() && layerdata[ii]->isNull()) {
            clearDur = true;
        }
    }

    if (dataindex.size() > 0) {
        prespace.resize(prespace.size() + 1);
        prespace.back() = layerendtime - startdur.back() - duration.back();
        if (clearDur) {
            prespace.back() = 0;
        }
    }

    // See https://github.com/humdrum-tools/verovio-humdrum-viewer/issues/124
    // This solution may need to be changed for a more general solution.
    for (int i = 0; i < (int)dataindex.size() - 1; ++i) {
        int ii = dataindex.at(i);
        int iii = dataindex.at(i + 1);
        if (prespace.at(ii) == 0) {
            continue;
        }
        if ((prespace.at(ii) + prespace.at(iii)) == 0) {
            prespace.at(ii) = 0;
            prespace.at(iii) = 0;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::hasFullMeasureRest -- Returns true if a single
//     rest in the measure which has the same duration as the
//     time signature.
//

bool HumdrumInput::hasFullMeasureRest(std::vector<hum::HTp> &layerdata, hum::HumNum timesigdur, hum::HumNum measuredur)
{
    if (timesigdur != measuredur) {
        return false;
    }
    int datacount = 0;
    for (int i = 0; i < (int)layerdata.size(); ++i) {
        hum::HTp token = layerdata[i];
        if (!token->isData()) {
            continue;
        }
        if (token->isNull()) {
            continue;
        }
        // deal with grace notes in same measure as mrest?
        datacount++;
        if (datacount > 1) {
            return false;
        }
        if (!token->isRest()) {
            return false;
        }
        if (token->getDurationFromBarline() > 0) {
            return false;
        }
        // Don't convert full-measure rests into spaces since
        // due to cases such as 5/4 measure rests.  Use @visible="false"
        // instead.
        // if (token->find("yy") != std::string::npos) {
        //    // treat invisible full-measure rest as a space later.
        //    return false;
        //}
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::appendElement --
//

template <class PARENT, class CHILD> void HumdrumInput::appendElement(PARENT parent, CHILD child)
{
    if (parent == NULL) {
        // probably a NULL measure, so store in section
        m_sections.back()->AddChild(child);
    }
    else {
        parent->AddChild(child);
    }
}

/////////////////////////////
//
// HumdrumInput::appendElement --
//

template <class CHILD>
void HumdrumInput::appendElement(const std::vector<std::string> &name, const std::vector<void *> &pointers, CHILD child)
{
    if (name.back() == "beam") {
        appendElement((Beam *)pointers.back(), child);
    }
    else if (name.back() == "gbeam") {
        appendElement((Beam *)pointers.back(), child);
    }
    else if (name.back() == "layer") {
        appendElement((Layer *)pointers.back(), child);
    }
    else if (name.back() == "tuplet") {
        appendElement((Tuplet *)pointers.back(), child);
    }
    else if (name.back() == "chord") {
        appendElement((Chord *)pointers.back(), child);
    }
    else if (name.back() == "ligature") {
        appendElement((Ligature *)pointers.back(), child);
    }
    else {
        std::cerr << "WARNING: Cannot append to unknown element: " << name.back() << std::endl;
    }
}

/////////////////////////////
//
// HumdrumInput::convertMRest --
//

void HumdrumInput::convertMRest(MRest *rest, hum::HTp token, int subtoken, int staffindex)
{

    std::string oloc = token->getValue("auto", "oloc");
    std::string ploc = token->getValue("auto", "ploc");
    int ottava = token->getValueInt("auto", "ottava");

    if ((!oloc.empty()) && (!ploc.empty())) {
        int olocint = stoi(oloc);
        olocint -= ottava;
        rest->SetOloc(olocint);
        if (ploc == "C") {
            rest->SetPloc(PITCHNAME_c);
        }
        else if (ploc == "D") {
            rest->SetPloc(PITCHNAME_d);
        }
        else if (ploc == "E") {
            rest->SetPloc(PITCHNAME_e);
        }
        else if (ploc == "F") {
            rest->SetPloc(PITCHNAME_f);
        }
        else if (ploc == "G") {
            rest->SetPloc(PITCHNAME_g);
        }
        else if (ploc == "A") {
            rest->SetPloc(PITCHNAME_a);
        }
        else if (ploc == "B") {
            rest->SetPloc(PITCHNAME_b);
        }
    }

    std::string tstring;
    if (subtoken < 0) {
        tstring = *token;
    }
    else {
        tstring = token->getSubtoken(subtoken);
    }

    if (tstring.find(";") != std::string::npos) {
        addFermata(rest, tstring);
    }
    processDynamics(token, staffindex);
    setLocationId(rest, token);
    if (m_doc->GetOptions()->m_humType.GetValue()) {
        embedQstampInClass(rest, token, *token);
    }

    if (token->find("yy") != std::string::npos) {
        rest->SetVisible(BOOLEAN_false);
    }
}

/////////////////////////////
//
// HumdrumInput::convertRest --
//

void HumdrumInput::convertRest(Rest *rest, hum::HTp token, int subtoken, int staffindex)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    // Shouldn't be in a chord, so add rest duration here.
    // Also full-measure rests are handled elsewhere.
    convertRhythm(rest, token, subtoken);

    std::string oloc = token->getValue("auto", "oloc");
    std::string ploc = token->getValue("auto", "ploc");
    int ottava = token->getValueInt("auto", "ottava");

    bool percussionQ = false;
    if (ss[staffindex].last_clef.compare(0, 6, "*clefX") == 0) {
        percussionQ = true;
    }

    if (percussionQ) {
        if ((!oloc.empty()) && (!ploc.empty())) {
            // treat as treble clef, but convert to @loc
            int olocint = stoi(oloc);
            olocint -= ottava;
            int plocint = 0;
            if (ploc == "C") {
                plocint = 0;
            }
            else if (ploc == "D") {
                plocint = 1;
            }
            else if (ploc == "E") {
                plocint = 2;
            }
            else if (ploc == "F") {
                plocint = 3;
            }
            else if (ploc == "G") {
                plocint = 4;
            }
            else if (ploc == "A") {
                plocint = 5;
            }
            else if (ploc == "B") {
                plocint = 6;
            }
            int loc = plocint + 7 * olocint;
            loc -= 30;
            rest->SetLoc(loc);
        }
    }
    else {
        if ((!oloc.empty()) && (!ploc.empty())) {
            int olocint = stoi(oloc);
            olocint -= ottava;
            rest->SetOloc(olocint);
            if (ploc == "C") {
                rest->SetPloc(PITCHNAME_c);
            }
            else if (ploc == "D") {
                rest->SetPloc(PITCHNAME_d);
            }
            else if (ploc == "E") {
                rest->SetPloc(PITCHNAME_e);
            }
            else if (ploc == "F") {
                rest->SetPloc(PITCHNAME_f);
            }
            else if (ploc == "G") {
                rest->SetPloc(PITCHNAME_g);
            }
            else if (ploc == "A") {
                rest->SetPloc(PITCHNAME_a);
            }
            else if (ploc == "B") {
                rest->SetPloc(PITCHNAME_b);
            }
        }
    }

    std::string tstring;
    if (subtoken < 0) {
        tstring = *token;
    }
    else {
        tstring = token->getSubtoken(subtoken);
    }
    // addDurRecip(rest, tstring);

    if (m_signifiers.above) {
        std::string pattern = "[ra-gA-G]+[-#nxXyY\\/]*";
        pattern.push_back(m_signifiers.above);
        if (regex_search(tstring, regex(pattern))) {
            int newstaff = m_currentstaff - 1;
            if ((newstaff > 0) && (newstaff <= (int)m_staffstarts.size())) {
                setStaff(rest, newstaff);
            }
        }
    }
    if (m_signifiers.below) {
        std::string pattern = "[ra-gA-G]+[-#nxXyY\\/]*";
        pattern.push_back(m_signifiers.below);
        if (regex_search(tstring, regex(pattern))) {
            int newstaff = m_currentstaff + 1;
            if ((newstaff > 0) && (newstaff <= (int)m_staffstarts.size())) {
                setStaff(rest, newstaff);
            }
        }
    }

    if (tstring.find(";") != std::string::npos) {
        addFermata(rest, tstring);
    }

    if (m_doc->GetOptions()->m_humType.GetValue()) {
        embedQstampInClass(rest, token, tstring);
    }

    // If the rest is the start or stop of an analytic phrase,
    // then color the rest (may change later, or be done with a label).
    bool phraseStart = token->find('{') != std::string::npos ? true : false;
    bool phraseStop = token->find('}') != std::string::npos ? true : false;
    if (phraseStart) {
        appendTypeTag(rest, "phraseStart");
    }
    if (phraseStop) {
        appendTypeTag(rest, "phraseStop");
    }

    token->setValue("MEI", "xml:id", rest->GetID());
    int index = (int)m_measures.size() - 1;
    token->setValue("MEI", "measureIndex", index);
}

//////////////////////////////
//
// HumdrumInput::checkForAutoStem -- For notes and chords (and possibly Rests).
//

template <class ELEMENT> void HumdrumInput::checkForAutoStem(ELEMENT element, hum::HTp token)
{
    int stemdir = token->getValueInt("auto", "stem.dir");
    if (stemdir == 1) {
        element->SetStemDir(STEMDIRECTION_up);
    }
    else if (stemdir == -1) {
        element->SetStemDir(STEMDIRECTION_down);
    }
}

//////////////////////////////
//
// HumdrumInput::setStemLength --
//

void HumdrumInput::setStemLength(Note *note, hum::HTp token)
{
    std::string stemlen = token->getValue("auto", "stemlen");
    if (stemlen.empty()) {
        return;
    }
    double value = std::stof(stemlen);
    note->SetStemLen(value);
}

/////////////////////////////
//
// HumdrumInput::convertNote --
//    default value:
//       subtoken = -1 (use the first subtoken);
//

void HumdrumInput::convertNote(Note *note, hum::HTp token, int staffadj, int staffindex, int subtoken)
{
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;

    if (subtoken <= 0) {
        if (token->find('H') != std::string::npos) {
            ss[staffindex].glissStarts.push_back(token);
        }
    }

    std::string tstring;
    int stindex = 0;
    if (subtoken < 0) {
        tstring = *token;
    }
    else {
        tstring = token->getSubtoken(subtoken);
        stindex = subtoken;
    }
    // addDurRecip(note, tstring);

    std::string scordaturaGes;
    if (!m_scordatura_marker.empty()) {
        scordaturaGes = checkNoteForScordatura(tstring);
    }

    bool chordQ = token->isChord();
    bool unpitchedQ = token->isUnpitched();
    if (chordQ) {
        // Allow rests in chords to be non-sounding fingered harmonic notes.
        // Need to check when a percussion clef.
        unpitchedQ = false;
        // SetVel for harmonics handled in checkForFingeredHarmonic()
        // bool isSilentQ = false;
        // if (tstring.find("r") != std::string::npos) {
        //     isSilentQ = true;
        // }
        // if (isSilentQ) {
        //     note->SetVel(0);
        // }
    }
    bool badpitchedQ = false;
    if (!unpitchedQ && (ss[staffindex].last_clef.compare(0, 6, "*clefX") == 0)) {
        badpitchedQ = true;
        unpitchedQ = true;
    }

    if (!chordQ) {
        setStemLength(note, token);
    }

    // Add beam break information:
    if (!token->isChord()) {
        int breaksec = token->getValueInt("", "auto", "breaksec");
        if (breaksec) {
            note->SetBreaksec(breaksec);
        }
    }

    if (!m_mens) {
        processTerminalLong(token); // do this before assigning rhythmic value.
        processTerminalBreve(token); // do this before assigning rhythmic value.
        processOverfillingNotes(token);
    }

    int line = token->getLineIndex();
    int field = token->getFieldIndex();
    colorNote(note, token, tstring, line, field);
    if (m_doc->GetOptions()->m_humType.GetValue()) {
        embedQstampInClass(note, token, tstring);
        embedPitchInformationInClass(note, tstring);
        embedTieInformation(note, tstring);
    }

    int ottava = token->getValueInt("auto", "ottava");
    bool octaveupQ = ottava == +1 ? true : false;
    bool octavedownQ = ottava == -1 ? true : false;
    bool octave2upQ = ottava == +2 ? true : false;
    bool octave2downQ = ottava == -2 ? true : false;

    if ((ss[staffindex].ottavameasure != NULL) && (ss[staffindex].ottavanotestart == NULL)) {
        ss[staffindex].ottavanotestart = note;
    }
    if ((ss[staffindex].ottavadownmeasure != NULL) && (ss[staffindex].ottavadownnotestart == NULL)) {
        ss[staffindex].ottavadownnotestart = note;
    }
    if ((ss[staffindex].ottava2measure != NULL) && (ss[staffindex].ottava2notestart == NULL)) {
        ss[staffindex].ottava2notestart = note;
    }
    if ((ss[staffindex].ottava2downmeasure != NULL) && (ss[staffindex].ottava2downnotestart == NULL)) {
        ss[staffindex].ottava2downnotestart = note;
    }

    hum::HumNum timestamp = 0;
    if (!m_mens) {
        hum::HumNum timestamp = token->getDurationFromStart();
    }

    if (ss[staffindex].ottavanoteend == NULL) {
        ss[staffindex].ottavanoteend = note;
    }
    else if (timestamp > ss[staffindex].ottavaendtimestamp) {
        ss[staffindex].ottavanoteend = note;
    }

    if (ss[staffindex].ottavadownnoteend == NULL) {
        ss[staffindex].ottavadownnoteend = note;
    }
    else if (timestamp > ss[staffindex].ottavadownendtimestamp) {
        ss[staffindex].ottavadownnoteend = note;
    }

    if (ss[staffindex].ottava2noteend == NULL) {
        ss[staffindex].ottava2noteend = note;
    }
    else if (timestamp > ss[staffindex].ottavaendtimestamp) {
        ss[staffindex].ottava2noteend = note;
    }

    if (ss[staffindex].ottava2downnoteend == NULL) {
        ss[staffindex].ottava2downnoteend = note;
    }
    else if (timestamp > ss[staffindex].ottavaendtimestamp) {
        ss[staffindex].ottava2downnoteend = note;
    }

    if (timestamp > ss[staffindex].ottavadownendtimestamp) {
        ss[staffindex].ottavadownnoteend = note;
    }
    if (timestamp > ss[staffindex].ottava2endtimestamp) {
        ss[staffindex].ottava2noteend = note;
    }
    if (timestamp > ss[staffindex].ottava2downendtimestamp) {
        ss[staffindex].ottava2downnoteend = note;
    }

    if (!chordQ) {
        if (tstring.find("qq") != std::string::npos) {
            note->SetGrace(GRACE_acc);
            // set the visual duration to an eighth note if there
            // is no rhythm specified (will be overwritten later
            // if there is a rhythm).
            note->SetDur(DURATION_8);
        }
        else if (tstring.find("q") != std::string::npos) {
            note->SetGrace(GRACE_unacc);
            // set the visual duration to an eighth note if there
            // is no rhythm specified (will be overwritten later
            // if there is a rhythm).
            note->SetDur(DURATION_8);
        }
    }

    // Add the pitch information
    hum::HumRegex hre;
    if (tstring.find("r") != std::string::npos) {
        hre.replaceDestructive(tstring, "", "r", "g");
    }
    int base40 = hum::Convert::kernToBase40(tstring);
    base40 += m_transpose[staffindex];
    int diatonic = hum::Convert::base40ToDiatonic(base40);
    int octave = diatonic / 7;

    int testaccid = 0;
    for (int i = 0; i < (int)tstring.size(); ++i) {
        if (tstring[i] == '-') {
            testaccid--;
        }
        else if (tstring[i] == '#') {
            testaccid++;
        }
    }
    if ((testaccid > 2) || (testaccid < -2)) {
        // reconsider notes that cannot be represented in base-40
        diatonic = -1;
        std::string dia;
        for (int i = 0; i < (int)tstring.size(); ++i) {
            switch (tstring[i]) {
                case 'c':
                case 'C':
                    diatonic = 0;
                    dia += tstring[i];
                    break;
                case 'd':
                case 'D':
                    diatonic = 1;
                    dia += tstring[i];
                    break;
                case 'e':
                case 'E':
                    diatonic = 2;
                    dia += tstring[i];
                    break;
                case 'f':
                case 'F':
                    diatonic = 3;
                    dia += tstring[i];
                    break;
                case 'g':
                case 'G':
                    diatonic = 4;
                    dia += tstring[i];
                    break;
                case 'a':
                case 'A':
                    diatonic = 5;
                    dia += tstring[i];
                    break;
                case 'b':
                case 'B':
                    diatonic = 6;
                    dia += tstring[i];
                    break;
            }
        }
        octave = (int)dia.size();
        if (octave > 0) {
            if (std::islower(dia[0])) {
                octave = octave + 3;
            }
            else {
                octave = 4 - octave;
            }
        }
    }

    if (!unpitchedQ) {
        if (octaveupQ) {
            // @oct required even if @oct.ges given.
            note->SetOctGes(octave);
            note->SetOct(octave - 1);
        }
        else if (octavedownQ) {
            // @oct required even if @oct.ges given.
            note->SetOctGes(octave);
            note->SetOct(octave + 1);
        }
        else if (octave2upQ) {
            // @oct required even if @oct.ges given.
            note->SetOctGes(octave);
            note->SetOct(octave - 2);
        }
        else if (octave2downQ) {
            // @oct required even if @oct.ges given.
            note->SetOctGes(octave);
            note->SetOct(octave + 2);
        }
        else {
            note->SetOct(octave);
        }
    }
    if (!unpitchedQ) {
        switch (diatonic % 7) {
            case 0: note->SetPname(PITCHNAME_c); break;
            case 1: note->SetPname(PITCHNAME_d); break;
            case 2: note->SetPname(PITCHNAME_e); break;
            case 3: note->SetPname(PITCHNAME_f); break;
            case 4: note->SetPname(PITCHNAME_g); break;
            case 5: note->SetPname(PITCHNAME_a); break;
            case 6: note->SetPname(PITCHNAME_b); break;
        }
    }

    if (unpitchedQ) {
        int loc = hum::Convert::kernToStaffLocation(token, "*clefX");
        note->SetLoc(loc);
        // suppress note@pname (see issue https://github.com/rism-digital/verovio/issues/1385)
        // suppress note@oct as well
    }

    if (badpitchedQ) {
        note->SetColor("#c41414");
    }

    // These three variables keep track of whether or not verovio is allowed
    // to convert the <accid> element into note@accid.  If there are attributes
    // to the accidental, any of these three can be set to true, which will
    // prevent the accid element from converting into an accid attribute.
    bool cautionaryQ = false;
    bool subelementQ = false;
    bool editorialQ = false;

    std::string edittype;
    if (!m_signifiers.editacc.empty()) {
        for (int x = 0; x < (int)m_signifiers.editacc.size(); ++x) {
            if (tstring.find(m_signifiers.editacc[x]) != std::string::npos) {
                editorialQ = true;
                edittype = m_signifiers.edittype[x];
                break;
            }
        }
    }
    std::string edittype2 = token->getLayoutParameter("A", "edit", subtoken);
    if (edittype.empty() && !edittype2.empty()) {
        editorialQ = true;
        if (edittype2 == "true") {
            // default editorial accidental type
            edittype = "";
            // use the first editorial accidental RDF style in file if present
            if (!m_signifiers.editacc.empty()) {
                edittype = m_signifiers.edittype[0];
            }
        }
        else {
            edittype = edittype2;
        }
    }

    int stemslashes = 0;
    if ((!token->isChord()) && m_signifiers.tremolo) {
        stemslashes = (int)std::count(tstring.begin(), tstring.end(), m_signifiers.tremolo);
    }

    bool removeStemQ = getBooleanParameter(token, "N", "xstem");
    bool addCueSizeQ = getBooleanParameter(token, "N", "cue");

    if (removeStemQ) {
        note->SetStemVisible(BOOLEAN_false);
    }
    if (addCueSizeQ) {
        note->SetCue(BOOLEAN_true);
    }
    if (stemslashes) {
        note->SetStemMod(note->AttStems::StrToStemmodifier(std::to_string(stemslashes) + "slash"));
    }

    std::string head = token->getLayoutParameter("N", "head", subtoken);
    if (head.empty()) {
        if (!ss[staffindex].m_notehead.empty()) {
            head = ss[staffindex].m_notehead;
        }
    }
    if (!head.empty()) {
        // See https://music-encoding.org/guidelines/v4/data-types/data.headshape.list.html
        // not all available in veorvio yet.
        if (head == "invis") {
            note->SetHeadVisible(BOOLEAN_false);
        }
        if (head == "x") {
            note->SetHeadShape(HEADSHAPE_x);
        }
        else if (head == "quarter") {
            note->SetHeadShape(HEADSHAPE_quarter);
        }
        else if (head == "solid") {
            note->SetHeadShape(HEADSHAPE_quarter);
        }
        else if (head == "open") {
            note->SetHeadShape(HEADSHAPE_half);
        }
        else if (head == "half") {
            note->SetHeadShape(HEADSHAPE_half);
        }
        else if (head == "whole") {
            note->SetHeadShape(HEADSHAPE_whole);
        }
        else if (head == "rhombus") {
            note->SetHeadShape(HEADSHAPE_diamond);
        }
        else if (head.compare(0, 3, "dia") == 0) {
            note->SetHeadShape(HEADSHAPE_diamond);
        }
        else if (head.compare(0, 4, "odia") == 0) {
            note->SetHeadShape(HEADSHAPE_diamond);
            note->SetHeadFill(FILL_void);
        }
        else if (head == "slash") {
            note->SetHeadShape(HEADSHAPE_slash);
        }
        else if (head == "plus") {
            note->SetHeadShape(HEADSHAPE_plus);
        }
        else if (head == "regular") {
            // do nothing, using default nohead-shape
        }
        else {
            // other unknown notehead shapes will also do nothing
        }
    }

    bool mensit = false;
    bool gesturalQ = false;
    bool hasAccidental = false;
    int accidlevel = 0;
    if (m_mens && token->isMensLike()) {
        // mensural notes are indicated differently, so check here for their method.
        if ((tstring.find("n") != std::string::npos) || (tstring.find("-") != std::string::npos)
            || (tstring.find("#") != std::string::npos)) {
            hasAccidental = true;
        }

        mensit = true;
        if (tstring.find("YY") != std::string::npos) {
            accidlevel = 1;
        }
        else if (tstring.find("Y") != std::string::npos) {
            accidlevel = 2;
        }
        else if (tstring.find("yy") != std::string::npos) {
            accidlevel = 3;
        }
        else if (tstring.find("y") != std::string::npos) {
            accidlevel = 4;
        }
        if (accidlevel <= ss[staffindex].acclev) {
            gesturalQ = false;
        }
        else {
            gesturalQ = true;
        }
    }

    Accid *accid = NULL;

    int accidCount = hum::Convert::base40ToAccidental(base40);
    if ((testaccid > 2) || (testaccid < -2)) {
        accidCount = testaccid;
    }
    // int accidCount = hum::Convert::kernToAccidentalCount(tstring);
    bool showInAccid = token->hasVisibleAccidental(stindex);
    bool showInAccidGes = !showInAccid;
    std::string loaccid = token->getLayoutParameter("N", "acc", subtoken);
    if (!loaccid.empty()) {
        // show the performance accidental in @accid.ges, and the
        // loaccid will be shown in @accid (the following false
        // will be used to set disable normal visual accid display system)
        showInAccid = false;
        showInAccidGes = true;
    }
    // alternate method of giving accidental
    std::string loaccid2 = token->getLayoutParameter("A", "vis", subtoken);
    if (!loaccid2.empty()) {
        // show the performance accidental in @accid.ges, and the
        // loaccid2 will be shown in @accid (the following false
        // will be used to set disable normal visual accid display system)
        showInAccid = false;
        showInAccidGes = true;
    }
    // loaccid2 has priority over loaccid when both present
    if (!loaccid2.empty()) {
        loaccid = loaccid2;
    }

    if (mensit && hasAccidental) {
        accid = new Accid();
        appendElement(note, accid);
        setLocationId(accid, token, subtoken);

        if (editorialQ) {
            accid->SetFunc(accidLog_FUNC_edit);
        }

        std::string color = token->getLayoutParameter("ACC", "color", subtoken);
        if (!color.empty()) {
            accid->SetColor(color);
            subelementQ = true;
        }

        if (gesturalQ) {
            switch (accidCount) {
                case +2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ss); break;
                case +1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_s); break;
                case 0: accid->SetAccidGes(ACCIDENTAL_GESTURAL_n); break;
                case -1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_f); break;
                case -2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ff); break;
            }
        }
        else {

            if (editorialQ) {

                accid->SetGlyphAuth("smufl");
                switch (accidCount) {
                    case +3:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_xs);
                        accid->SetGlyphName("accidentalTripleSharp");
                        break;
                    case +2:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_x);
                        accid->SetGlyphName("accidentalDoubleSharp");
                        break;
                    case +1:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_s);
                        accid->SetGlyphName("accidentalSharp");
                        break;
                    case 0:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_n);
                        accid->SetGlyphName("accidentalNatural");
                        break;
                    case -1:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_f);
                        accid->SetGlyphName("accidentalFlat");
                        break;
                    case -2:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_ff);
                        accid->SetGlyphName("accidentalDoubleFlat");
                        break;
                    case -3:
                        accid->SetAccid(ACCIDENTAL_WRITTEN_tf);
                        accid->SetGlyphName("accidentalTripleFlat");
                        break;
                }
            }
            else {

                switch (accidCount) {
                    case +3: accid->SetAccid(ACCIDENTAL_WRITTEN_xs); break;
                    case +2: accid->SetAccid(ACCIDENTAL_WRITTEN_x); break;
                    case +1: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break;
                    case 0:
                        // mensural music does not have a natural sign
                        // and accidentals are relative
                        switch (diatonic % 7) {
                            case 0: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break; // C# -> Cn
                            case 1: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break; // D# -> Dn
                            case 2: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break; // E- -> En
                            case 3: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break; // F# -> Fn
                            case 4: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break; // G# -> Gn
                            case 5: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break; // A- -> An
                            case 6: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break; // B- -> Bn
                        }
                        break;
                    case -1: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break;
                    case -2: accid->SetAccid(ACCIDENTAL_WRITTEN_ff); break;
                    case -3: accid->SetAccid(ACCIDENTAL_WRITTEN_tf); break;
                    default: std::cerr << "Do not know how to convert accidental: " << accidCount << endl;
                }

                if (accidlevel != 0) {
                    accid->SetFunc(accidLog_FUNC_edit);
                }
            }
        }
    }
    else if (!mensit && !unpitchedQ) {
        accid = new Accid();
        appendElement(note, accid);
        setLocationId(accid, token, subtoken);

        std::string color = token->getLayoutParameter("ACC", "color", subtoken);
        if (!color.empty()) {
            accid->SetColor(color);
            subelementQ = true;
        }

        if (!editorialQ) {
            // don't mark cautionary accidentals if the note has
            // an editorial accidental.
            if (token->hasCautionaryAccidental(stindex)) {
                addCautionaryAccidental(accid, token, accidCount);
                cautionaryQ = true;
                showInAccidGes = false;
                showInAccid = false;
            }
        }

        if (!editorialQ) {
            if (showInAccid) {
                switch (accidCount) {
                    case +3: accid->SetAccid(ACCIDENTAL_WRITTEN_xs); break;
                    case +2: accid->SetAccid(ACCIDENTAL_WRITTEN_x); break;
                    case +1: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break;
                    case 0: accid->SetAccid(ACCIDENTAL_WRITTEN_n); break;
                    case -1: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break;
                    case -2: accid->SetAccid(ACCIDENTAL_WRITTEN_ff); break;
                    case -3: accid->SetAccid(ACCIDENTAL_WRITTEN_tf); break;
                    default: std::cerr << "Do not know how to convert accidental: " << accidCount << endl;
                }
            }
            else if (!loaccid.empty()) {
                if (loaccid == "n#") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_ns);
                }
                else if (loaccid == "#") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_s);
                }
                else if (loaccid == "n") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_n);
                }
                else if (loaccid == "##") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_ss);
                }
                else if (loaccid == "x") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_x);
                }
                else if (loaccid == "-") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_f);
                }
                else if (loaccid == "--") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_ff);
                }
                else if (loaccid == "#x") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_sx);
                }
                else if (loaccid == "###") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_ts);
                }
                else if (loaccid == "n-") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_nf);
                }
                else if (loaccid == "---") {
                    accid->SetAccid(ACCIDENTAL_WRITTEN_tf);
                }
                else {
                    std::cerr << "Warning: unknown accidental type " << std::endl;
                }
                // add more accidentals here as necessary.  Mostly left are quarter tones
                // which are not dealt with directly in **kern data: su, sd, fu, fd, nu,
                // nd, 1qf, 3qf, 1qs, 3qs
                // http://music-encoding.org/guidelines/v3/data-types/data.accidental.explicit.html
            }
        }
        else {
            if (edittype == "") {
                accid->SetFunc(accidLog_FUNC_edit);
            }
            else if (edittype.find("above") != std::string::npos) {
                accid->SetFunc(accidLog_FUNC_edit);
            }
            else if (edittype == "a") {
                accid->SetFunc(accidLog_FUNC_edit);
            }
            else if (edittype.find("up") != std::string::npos) {
                accid->SetFunc(accidLog_FUNC_edit);
            }
            if (edittype.find("brack") != std::string::npos) {
                // enclose="brack" cannot be present with func="edit" at the moment...
                accid->SetEnclose(ENCLOSURE_brack);
            }
            else if (edittype.find("brac") != std::string::npos) {
                // enclose="brac" cannot be present with func="edit" at the moment...
                accid->SetEnclose(ENCLOSURE_brack);
            }
            if (edittype.find("paren") != std::string::npos) {
                // enclose="paren" cannot be present with func="edit" at the moment...
                accid->SetEnclose(ENCLOSURE_paren);
            }
            else if (edittype.find("none") != std::string::npos) {
                // display as a regular accidental
            }
            if (loaccid.empty()) {
                switch (accidCount) {
                    case +2: accid->SetAccid(ACCIDENTAL_WRITTEN_x); break;
                    case +1: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break;
                    case 0: accid->SetAccid(ACCIDENTAL_WRITTEN_n); break;
                    case -1: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break;
                    case -2: accid->SetAccid(ACCIDENTAL_WRITTEN_ff); break;
                }
            }
            else {
                setAccid(accid, loaccid);
            }
        }

        if (showInAccidGes) {
            if (loaccid.empty()) {
                switch (accidCount) {
                    // case +3: note->SetAccidGes(ACCIDENTAL_GESTURAL_ts); break;
                    // case -3: note->SetAccidGes(ACCIDENTAL_GESTURAL_tf); break;
                    case +2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ss); break;
                    case +1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_s); break;
                    case 0: accid->SetAccidGes(ACCIDENTAL_GESTURAL_n); break;
                    case -1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_f); break;
                    case -2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ff); break;
                }
            }
        }

        if (!(editorialQ || cautionaryQ || subelementQ)) {
            // No need for sub-element so make them attributes of the note:
            accid->IsAttribute(true);
        }
    }

    if (!chordQ) {
        hum::HumNum dur;
        if (m_hasTremolo && token->getValueBool("auto", "tremolo")) {
            hum::HumdrumToken newtok(token->getValue("auto", "recip"));
            dur = convertRhythm(note, &newtok, 0);
        }
        else if (m_hasTremolo && token->getValueBool("auto", "tremolo2")) {
            hum::HumdrumToken newtok(token->getValue("auto", "recip"));
            dur = convertRhythm(note, &newtok, 0);
        }
        else if (m_hasTremolo && token->getValueBool("auto", "tremoloAux")) {
            hum::HumdrumToken newtok(token->getValue("auto", "recip"));
            dur = convertRhythm(note, &newtok, 0);
        }
        else {
            dur = convertRhythm(note, token, subtoken);
            // duration not set here because probably already
            // done at line eDcRfV
        }
        if (m_setrightstem) {
            m_setrightstem = false;
            note->SetStemPos(STEMPOSITION_right);
        }
        if (dur == 0) {
            note->SetDur(DURATION_4);
            note->SetStemVisible(BOOLEAN_false);
            // if you want a stemless grace note, then set the
            // stemlength to zero explicitly.
        }

        checkForJoin(note, token);
    }
    else {
        // deal with visual rhythms on a note that are different from the chord
        std::string chordvis = token->getVisualDurationChord();
        if (chordvis.empty()) {
            std::string notevis = token->getVisualDuration(subtoken);
            if (!notevis.empty()) {
                convertRhythm(note, token, subtoken);
            }
        }
    }

    if (!chordQ) {
        if (tstring.find("/") != std::string::npos) {
            note->SetStemDir(STEMDIRECTION_up);
            appendTypeTag(note, "placed");
        }
        else if (tstring.find("\\") != std::string::npos) {
            note->SetStemDir(STEMDIRECTION_down);
            appendTypeTag(note, "placed");
        }
        checkForAutoStem(note, token);
    }

    if (!mensit) {
        // yy means make invisible in **kern, but is used for accidental levels in **mens.
        if (tstring.find("yy") != std::string::npos) {
            note->SetVisible(BOOLEAN_false);
        }
    }

    // alterted notes (MEI 5):
    if (mensit) {
        addMensuralQuality(note, token);
    }

    if (!mensit) {
        if (tstring.find("P") != std::string::npos) {
            appendTypeTag(note, "appoggiatura-start");
        }
        if (tstring.find("p") != std::string::npos) {
            appendTypeTag(note, "appoggiatura-stop");
        }
    }

    // handle ties
    if (!token->isMensLike()) {
        if ((tstring.find("[") != std::string::npos) || (tstring.find("_") != std::string::npos)) {
            processTieStart(note, token, tstring, subtoken);
        }

        if ((tstring.find("_") != std::string::npos) || (tstring.find("]") != std::string::npos)) {
            processTieEnd(note, token, tstring, subtoken);
        }
    }

    if (m_signifiers.above) {
        std::string pattern = "[ra-gA-G]+[-#nxXyY]*";
        pattern.push_back(m_signifiers.above);
        if (regex_search(tstring, regex(pattern))) {
            int newstaff = m_currentstaff - 1;
            if ((staffadj == 0) && (newstaff > 0) && (newstaff <= (int)m_staffstarts.size())) {
                setStaff(note, newstaff);
            }
        }
    }
    if (m_signifiers.below) {
        std::string pattern = "[ra-gA-G]+[-#nxXyY]*";
        pattern.push_back(m_signifiers.below);
        if (regex_search(tstring, regex(pattern))) {
            int newstaff = m_currentstaff + 1;
            if ((staffadj == 0) && (newstaff > 0) && (newstaff <= (int)m_staffstarts.size())) {
                setStaff(note, newstaff);
            }
        }
    }

    if (subtoken < 0) {
        convertVerses(note, token);
    }

    // maybe organize by sub-token index, but consider as chord for now
    if (!chordQ) {
        token->setValue("MEI", "xml:id", note->GetID());
        int index = (int)m_measures.size() - 1;
        token->setValue("MEI", "measureIndex", index);
    }

    // check for cue-size signifier:
    if (m_signifiers.cuesize && tstring.find(m_signifiers.cuesize) != std::string::npos) {
        note->SetCue(BOOLEAN_true);
    }
    else if (m_staffstates.at(staffindex).cue_size.at(m_currentlayer)) {
        note->SetCue(BOOLEAN_true);
    }

    // If the note is the start or stop of an analytic phrase,
    // then color the note (may change later, or be done with a label).
    bool phraseStart = token->find('{') != std::string::npos ? true : false;
    bool phraseStop = token->find('}') != std::string::npos ? true : false;
    if (phraseStart) {
        appendTypeTag(note, "phraseStart");
    }
    if (phraseStop) {
        appendTypeTag(note, "phraseStop");
    }

    if (!scordaturaGes.empty()) {
        hum::HumPitch hpitch;
        hpitch.setKernPitch(scordaturaGes);
        int oct = hpitch.getOctave();
        note->SetOctGes(oct);
        switch (hpitch.getDiatonicPC()) {
            case 0: note->SetPnameGes(PITCHNAME_c); break;
            case 1: note->SetPnameGes(PITCHNAME_d); break;
            case 2: note->SetPnameGes(PITCHNAME_e); break;
            case 3: note->SetPnameGes(PITCHNAME_f); break;
            case 4: note->SetPnameGes(PITCHNAME_g); break;
            case 5: note->SetPnameGes(PITCHNAME_a); break;
            case 6: note->SetPnameGes(PITCHNAME_b); break;
        }
        // note@accid.ges is likely to be overwritten, but this is needed for
        // correct MIDI output.
        if (accid) {
            switch (hpitch.getAccid()) {
                case +2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ss); break;
                case +1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_s); break;
                case 0: accid->SetAccidGes(ACCIDENTAL_GESTURAL_n); break;
                case -1: accid->SetAccidGes(ACCIDENTAL_GESTURAL_f); break;
                case -2: accid->SetAccidGes(ACCIDENTAL_GESTURAL_ff); break;
            }
        }
        appendTypeTag(note, "scoredatura");
    }
}

//////////////////////////////
//
// HumdrumInput::checkForJoin -- Assuming only two layers for joining for now.
//

bool HumdrumInput::checkForJoin(Note *note, hum::HTp token)
{
    if (!m_join) {
        return false;
    }
    if (token->isChord()) {
        // Don't join chords.
        return false;
    }
    if (token->isRest()) {
        // Deal with rests later (and can't be input to this function anyway).
        return false;
    }
    int subtrack = token->getSubtrack();
    if (subtrack != 2) {
        // Only applies to second layer.  Add higher layers later.
        return false;
    }
    if (token->getValueBool("auto", "Xjoin")) {
        return false;
    }
    int track = token->getTrack();
    hum::HTp ptok = token->getPreviousFieldToken();
    if (!ptok) {
        return false;
    }
    if (ptok->isChord() || ptok->isRest() || ptok->isNull()) {
        return false;
    }
    int ptrack = ptok->getTrack();
    if (ptrack != track) {
        return false;
    }

    hum::HumNum dur = token->getDuration();
    hum::HumNum pdur = ptok->getDuration();

    int b40 = token->getBase40Pitch();
    int pb40 = ptok->getBase40Pitch();

    if (dur == pdur) {
        if (b40 == pb40) {
            // same pitch so make the entire notes the same:
            std::string pid = getLocationId(note, ptok);
            note->SetSameas("#" + pid);
            return true;
        }
        else {
            // same duration but different pitch:
            // will be put into a chord by verovio
            std::string pid = getLocationId(note, ptok);
            note->SetStemSameas("#" + pid);
            return true;
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::addMensuralQuality -- Add explicit @num and @numbase for mensural notes
//     that doe not match the mensuration.  This can be removed later when verovio does
//     this on its own.
//
//     maximodus   splits maxima    into 2 or 3 longas
//     modus       splits longa     into 2 or 3 breves
//     tempus      splits breve     into 2 or 3 semibreves
//     prolatio    splits semibreve into 2 or 3 minims
//
// To do: add rhythmic scaling
//

void HumdrumInput::addMensuralQuality(Note *note, hum::HTp token)
{
    if (token->find("+") != std::string::npos) {
        note->SetDurQuality(DURQUALITY_mensural_altera);
        return;
    }

    // Check for explicit perfection/imperfection:
    bool perfect = token->find("p") == std::string::npos ? false : true;
    bool imperfect = token->find("i") == std::string::npos ? false : true;

    // What rhythmic level is the note:
    bool maxima = token->find("X") == std::string::npos ? false : true;
    bool longa = token->find("L") == std::string::npos ? false : true;
    bool breve = token->find("S") == std::string::npos ? false : true;
    bool semibreve = token->find("s") == std::string::npos ? false : true;

    if (!(maxima || longa || breve || semibreve)) {
        // minim, semiminim, fusa, and semifusa should always be imperfect
        return;
    }

    // Do not put @num/@numbase on notes/rests that match the mensuration:
    int staffindex = m_currentstaff - 1;
    humaux::StaffStateVariables &ss = m_staffstates.at(staffindex);
    if (maxima && perfect && (ss.maximodus == 3)) {
        return;
    }
    else if (maxima && imperfect && (ss.maximodus == 2)) {
        return;
    }
    else if (longa && perfect && (ss.modus == 3)) {
        return;
    }
    else if (longa && imperfect && (ss.modus == 2)) {
        return;
    }
    else if (breve && perfect && (ss.tempus == 3)) {
        return;
    }
    else if (breve && imperfect && (ss.tempus == 2)) {
        return;
    }
    else if (semibreve && perfect && (ss.prolatio == 3)) {
        return;
    }
    else if (semibreve && imperfect && (ss.prolatio == 2)) {
        return;
    }

    // Mark note/rest as perfect/imperfect:
    if (token->find("i") != std::string::npos) {
        note->SetDurQuality(DURQUALITY_mensural_imperfecta);
        // imperfect time adjustment:
        note->SetNum(3);
        note->SetNumbase(2);
    }
    if (token->find("p") != std::string::npos) {
        note->SetDurQuality(DURQUALITY_mensural_perfecta);
        // perfect time adjustment:
        note->SetNum(2);
        note->SetNumbase(3);
    }
}

//////////////////////////////
//
// HumdrumInput::addDurRecip -- Add the **recip duration of the note/rest
//     as `@dur.recip`.  Only numeric **recip values are used, so breves
//     encoded in a Humdrum file as "0" are converted to the numeric
//     equivalent of "1%2".  Grace notes are then given a numeric value
//     of "0", which is otherwise represented by "q" in Humdrum data.
//     The augmentation dots position can be malformed and possibly
//     placed elsewhere, but the function will not currently handle
//     such cases.
//

template <class ELEMENT> void HumdrumInput::addDurRecip(ELEMENT element, const std::string &ttoken)
{
    if (ttoken.find('q') != std::string::npos) {
        element->SetDurRecip("0");
        return;
    }
    hum::HumRegex hre;
    if (!hre.search(ttoken, "([0-9]+(?:%[0-9]+)?\\.*)")) {
        // This is possible in an alternate compressed
        // chord representation where the secondary
        // notes in the chord do not have explicit durations.
        // Currently ignore such cases.
        return;
    }
    std::string recip = hre.getMatch(1);
    if (hre.search(recip, "^(0+)")) {
        // Convert symbolic rhythms to numeric equivalents
        std::string zeros = hre.getMatch(1);
        int zcount = (int)zeros.size();
        int value = (int)pow(2.0, zcount);
        std::string replacement = "1%";
        replacement += to_string(value);
        hre.replaceDestructive(recip, replacement, "^0+");
    }
    element->SetDurRecip(recip);
}

//////////////////////////////
//
// HumdrumInput::appendTypeTag -- add a type to an MEI element.  Appends to the
//    current type if there is already any type contents.
//

template <class ELEMENT> void HumdrumInput::appendTypeTag(ELEMENT *element, const std::string &aType)
{
    std::string currentType = element->GetType();
    if (currentType.empty()) {
        element->SetType(aType);
    }
    else {
        currentType += " ";
        currentType += aType;
        element->SetType(currentType);
    }
}

//////////////////////////////
//
// HumdrumInput::checkNoteForScordatura -- Return the **kern written note if scordatura; otherwise, return empty string.
//

std::string HumdrumInput::checkNoteForScordatura(const std::string &token)
{
    int index = -1;
    for (int i = 0; i < (int)m_scordatura_marker.size(); ++i) {
        if (token.find(m_scordatura_marker[i]) != std::string::npos) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return "";
    }
    hum::HumPitch tpitch;
    tpitch.setKernPitch(token);
    m_scordatura_transposition[index]->transpose(tpitch);
    return tpitch.getKernPitch();
}

//////////////////////////////
//
// HumdrumInput::addCautionaryAccidental --
//

void HumdrumInput::addCautionaryAccidental(Accid *accid, hum::HTp token, int acount)
{
    accid->SetFunc(accidLog_FUNC_caution);
    switch (acount) {
        case +3: accid->SetAccid(ACCIDENTAL_WRITTEN_ts); break;
        case +2: accid->SetAccid(ACCIDENTAL_WRITTEN_x); break;
        case +1: accid->SetAccid(ACCIDENTAL_WRITTEN_s); break;
        case 0: accid->SetAccid(ACCIDENTAL_WRITTEN_n); break;
        case -1: accid->SetAccid(ACCIDENTAL_WRITTEN_f); break;
        case -2: accid->SetAccid(ACCIDENTAL_WRITTEN_ff); break;
        case -3: accid->SetAccid(ACCIDENTAL_WRITTEN_tf); break;
    }
}

//////////////////////////////
//
// HumdrumInput::setAccid -- set the written accidental according
//    to the given style.
//

void HumdrumInput::setAccid(Accid *accid, const std::string &loaccid)
{
    if (loaccid.empty()) {
        return;
    }
    if (loaccid == "#") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_s);
    }
    else if (loaccid == "-") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_f);
    }
    else if (loaccid == "##") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_ss);
    }
    else if (loaccid == "x") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_x);
    }
    else if (loaccid == "--") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_ff);
    }
    else if (loaccid == "xs") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_xs);
    }
    else if (loaccid == "sx") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_sx);
    }
    else if (loaccid == "###") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_ts);
    }
    else if (loaccid == "---") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_ts);
    }
    else if (loaccid == "n") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_n);
    }
    else if (loaccid == "n-") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_nf);
    }
    else if (loaccid == "n#") {
        accid->SetAccid(ACCIDENTAL_WRITTEN_ns);
    }
}

//////////////////////////////
//
// HumdrumInput::getVerseLabels --
//

std::vector<hum::HTp> HumdrumInput::getVerseLabels(hum::HTp token, int staff)
{
    std::vector<hum::HTp> output;
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    if (ss[staff].verse_labels.empty()) {
        return output;
    }
    std::vector<hum::HTp> remainder;
    std::string spineinfo = token->getSpineInfo();

    for (int i = 0; i < (int)ss[staff].verse_labels.size(); ++i) {
        if (ss[staff].verse_labels[i]->getSpineInfo() == spineinfo) {
            output.push_back(ss[staff].verse_labels[i]);
        }
        else {
            remainder.push_back(ss[staff].verse_labels[i]);
        }
    }

    if (output.empty()) {
        return output;
    }
    else {
        ss[staff].verse_labels = remainder;
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::getVerseAbbrLabels --
//

std::vector<hum::HTp> HumdrumInput::getVerseAbbrLabels(hum::HTp token, int staff)
{
    std::vector<hum::HTp> output;
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    if (ss[staff].verse_abbr_labels.empty()) {
        return output;
    }
    std::vector<hum::HTp> remainder;
    std::string spineinfo = token->getSpineInfo();
    for (int i = 0; i < (int)ss[staff].verse_abbr_labels.size(); ++i) {
        if (ss[staff].verse_abbr_labels[i]->getSpineInfo() == spineinfo) {
            output.push_back(ss[staff].verse_abbr_labels[i]);
        }
        else {
            remainder.push_back(ss[staff].verse_abbr_labels[i]);
        }
    }
    if (output.empty()) {
        return output;
    }
    else {
        ss[staff].verse_abbr_labels = remainder;
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::convertVerses --
//

template <class ELEMENT> void HumdrumInput::convertVerses(ELEMENT element, hum::HTp token)
{
    int staff = m_rkern[token->getTrack()];
    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    if (!ss[staff].verse) {
        return;
    }

    int subtrack = token->getSubtrack();
    if (subtrack > 1) {
        if (token->noteInLowerSubtrack()) {
            // don't print a lyric for secondary layers unless
            // all of the lower layers do not have a note attacking
            // or tied at the same time.  This is because verovio
            // will incorrectly overstrike syllables shared between
            // layers if there is an offset of a second between the layers.
            return;
            // probably also have to deal with chords containing seconds...
        }
    }

    hum::HumRegex hre;
    std::vector<std::string> vtexts;
    std::vector<hum::HTp> vtoks;
    hum::HTp vtoken = NULL;
    std::string content;
    hum::HumdrumLine &line = *token->getLine();
    int track = token->getTrack();
    int ttrack;
    int versenum = 0;
    bool vvdataQ;
    bool vdataQ;
    bool lyricQ;
    int startfield = token->getFieldIndex() + 1;
    for (int i = startfield; i < line.getFieldCount(); ++i) {
        hum::HTp token = line.token(i);
        std::string exinterp = token->getDataType();

        if (token->isKernLike()) {
            ttrack = token->getTrack();
            if (ttrack != track) {
                break;
            }
        }

        if (token->isMensLike()) {
            ttrack = token->getTrack();
            if (ttrack != track) {
                break;
            }
        }

        lyricQ = false;
        vdataQ = false;
        vvdataQ = false;
        if (token->isDataTypeLike("**text")) {
            lyricQ = true;
        }
        else if (token->isDataTypeLike("**silbe")) {
            lyricQ = true;
        }
        else if (token->getDataType().compare(0, 7, "**vdata") == 0) {
            vdataQ = true;
            lyricQ = true;
        }
        else if (token->getDataType().compare(0, 8, "**vvdata") == 0) {
            vvdataQ = true;
            lyricQ = true;
        }

        if (!lyricQ) {
            continue;
        }

        if (token->isNull()) {
            versenum++;
            continue;
        }
        if (token->isDataTypeLike("**silbe")) {
            if (token->getText() == "|") {
                versenum++;
                continue;
            }
        }

        std::vector<hum::HTp> labels;
        std::string verselabel;

        if (!ss[staff].verse_labels.empty()) {
            labels = getVerseLabels(token, staff);
            if (!labels.empty()) {
                verselabel = getVerseLabelText(labels[0]);
            }
        }

        std::vector<hum::HTp> abbrlabels;
        std::string verseabbrlabel;
        if (!ss[staff].verse_abbr_labels.empty()) {
            abbrlabels = getVerseAbbrLabels(token, staff);
            if (!abbrlabels.empty()) {
                verseabbrlabel = getVerseLabelText(abbrlabels[0]);
            }
        }

        vtexts.clear();
        vtoks.clear();
        // int track = token->getTrack();
        // int strack = token->getSubtrack();
        if (token->isDataTypeLike("**silbe")) {
            vtoks.push_back(token);
            std::string value = token->getValue("auto", "text");
            if (value.empty()) {
                value = *token;
            }
            hre.replaceDestructive(value, "", "\\|", "g");
            hre.replaceDestructive(value, "&#xFC;", "u2", "g"); // u-umlaut
            hre.replaceDestructive(value, "&#xE4;", "a2", "g"); // a-umlaut
            hre.replaceDestructive(value, "&#xF6;", "o2", "g"); // o-umlaut
            vtexts.push_back(value);
        }
        else {
            vtoks.push_back(token);
            std::string value = token->getValue("auto", "text");
            if (value.empty()) {
                value = *token;
            }
            vtexts.push_back(value);
        }
        if (vvdataQ) {
            splitSyllableBySpaces(vtexts);
        }

        for (int j = 0; j < (int)vtexts.size(); ++j) {
            content = vtexts[j];
            vtoken = vtoks[j];
            versenum++;
            if (content == "") {
                continue;
            }

            Verse *verse = new Verse();

            std::string color = vtoken->getValue("auto", "color");
            if (color == "black") {
                color = "";
            }
            else if (color == "#000") {
                color = "";
            }
            else if (color == "#000000") {
                color = "";
            }

            if (!color.empty()) {
                verse->SetColor(color);
            }

            if (vvdataQ) {
                setLocationId(verse, token, j + 1);
            }
            else {
                setLocationId(verse, token, -1);
            }
            appendElement(element, verse);
            verse->SetN(versenum);

            if (!verselabel.empty()) {
                Label *label = new Label();
                Text *text = new Text();
                std::u32string wtext = UTF8to32(verselabel);
                text->SetText(wtext);
                verse->AddChild(label);
                label->AddChild(text);
            }
            if (!verseabbrlabel.empty()) {
                LabelAbbr *labelabbr = new LabelAbbr();
                Text *text = new Text();
                std::u32string wtext = UTF8to32(verseabbrlabel);
                text->SetText(wtext);
                verse->AddChild(labelabbr);
                labelabbr->AddChild(text);
            }

            Syl *syl = new Syl();
            std::vector<Syl *> syls; // verse can have multiple syls if elision(s) present
            syls.push_back(syl);
            appendElement(verse, syls.back());

            std::string datatype = token->getDataType();

            if (datatype.compare(0, 8, "**vdata-") == 0) {
                std::string subdatatype = datatype.substr(8);
                if (!subdatatype.empty()) {
                    appendTypeTag(syls.back(), subdatatype);
                }
            }
            else if (datatype.compare(0, 9, "**vdata-") == 0) {
                std::string subdatatype = datatype.substr(9);
                if (!subdatatype.empty()) {
                    appendTypeTag(syls.back(), subdatatype);
                }
            }

            // add IDs for first syb-syllable:
            if (vvdataQ) {
                setLocationId(syls.back(), token, j + 1);
            }
            else {
                setLocationId(syls.back(), token, -1);
            }

            if (vdataQ || vvdataQ) {
                // do not treat text content as lyrics
                addTextElement(syls.back(), content);
                continue;
            }

            colorVerse(verse, content);

            bool dashonbegin = false;
            bool dashonend = false;
            bool extender = false;

            std::vector<std::string> contents(1);

            // split syllable by elisions:
            contents[0] += content[0];
            for (int z = 1; z < (int)content.size() - 1; ++z) {
                // Use underscore for elision symbol.
                // Now using @con="b" when verovio allows it.
                // Also possibly make elision symbols optional.
                if ((content[z] == ' ') && (content[z + 1] != '\'')) {
                    // the later condition is to not to elide "ma 'l"
                    // create an elision by separating into next piece of syllable
                    contents.resize(contents.size() + 1);
                    // content[z] = '_';
                }
                else {
                    contents.back() += content[z];
                }
            }
            if (content.size() > 1) {
                contents.back() += content.back();
            }

            // add elements for sub-syllables due to elisions:
            for (int k = 1; k < (int)contents.size(); k++) {
                Syl *syl = new Syl();
                syls.push_back(syl);
                appendElement(verse, syl);
            }
            // Connect all sub-syllables except last as elisions.
            // elision character styles:
            // @con="t" : tilde
            // @con="c" : circumflex
            // @con="v" : caron
            // @con="i" : inverted breve (curved line above)
            // @con="b" : breve (curved line below)
            // no space connector?
            for (int k = 0; k < (int)contents.size() - 1; k++) {
                syls[k]->SetCon(sylLog_CON_b);
            }
            // add sub-syllables to verse:

            if (content.back() == '-') { // d connector
                dashonend = true;
                content.pop_back();
                contents.back().pop_back();
            }
            if ((content.size() > 0) && (content[0] == '-')) {
                dashonbegin = true;
                content.erase(0, 1);
                contents[0].erase(0, 1);
            }
            if (content.back() == '_') { // u connector
                extender = true;
                content.pop_back();
                contents.back().pop_back();
            }

            // @wordpos="i" : syllable at start of word (initial)
            // @wordpos="m" : syllable in middle of word
            // @wordpos="t" : syllable at end of word (terminal)
            // nothing: syllable is a word

            if (dashonbegin && dashonend) {
                if (syls.size() > 1) {
                    syls[0]->SetWordpos(sylLog_WORDPOS_t);
                    syls.back()->SetWordpos(sylLog_WORDPOS_i);
                    syls.back()->SetCon(sylLog_CON_d);
                    if (m_doc->GetOptions()->m_humType.GetValue()) {
                        appendTypeTag(syls[0], "t");
                        appendTypeTag(syls.back(), "i");
                    }
                }
                else {
                    syls.back()->SetWordpos(sylLog_WORDPOS_m);
                    syls.back()->SetCon(sylLog_CON_d);
                    if (m_doc->GetOptions()->m_humType.GetValue()) {
                        appendTypeTag(syls.back(), "m");
                    }
                }
            }
            else if (dashonbegin) {
                syls[0]->SetWordpos(sylLog_WORDPOS_t);
                if (m_doc->GetOptions()->m_humType.GetValue()) {
                    appendTypeTag(syls[0], "t");
                }
            }
            else if (dashonend) {
                syls.back()->SetWordpos(sylLog_WORDPOS_i);
                syls.back()->SetCon(sylLog_CON_d);
                if (m_doc->GetOptions()->m_humType.GetValue()) {
                    appendTypeTag(syls.back(), "i");
                }
            }
            else {
                if (m_doc->GetOptions()->m_humType.GetValue()) {
                    appendTypeTag(syls[0], "t");
                }
            }
            if (extender) {
                syls.back()->SetWordpos(sylLog_WORDPOS_t);
                syls.back()->SetCon(sylLog_CON_u);
                if (m_doc->GetOptions()->m_humType.GetValue()) {
                    appendTypeTag(syls.back(), "t");
                }
            }
            // remove the last dash in a line (double dash which indicates
            // a dash in the original word separator).
            if ((!content.empty()) && content.back() == '-') {
                content.resize(content.size() - 1);
            }
            if (!contents.back().empty() && contents.back().back() == '-') {
                contents.back().resize(contents.back().size() - 1);
            }

            std::string inij = vtoken->getValue("auto", "ij");
            bool ij = !inij.empty();

            for (int m = 0; m < (int)contents.size(); m++) {
                if (m > 0) {
                    std::string id = syls[0]->GetID();
                    id += "S" + to_string(m + 1);
                    syls[m]->SetID(id);
                }
                bool spacer = false;
                if ((contents.size() == 1) && (contents[0].size() == 1)) {
                    spacer = true;
                }

                if (ij) {
                    Rend *rend = new Rend();
                    rend->SetFontstyle(FONTSTYLE_italic);
                    addTextElement(rend, contents[m], "", spacer);
                    syls[m]->AddChild(rend);
                    std::string ijbegin = vtoken->getValue("auto", "ij-begin");
                    bool ijbeginQ = !ijbegin.empty();
                    std::string ijend = vtoken->getValue("auto", "ij-end");
                    bool ijendQ = !ijend.empty();
                    if (ijbeginQ && ijendQ) {
                        syls[m]->SetType("repetition repetition-begin repetition-end");
                    }
                    else if (ijbeginQ && (m == 0)) {
                        syls[m]->SetType("repetition repetition-begin");
                    }
                    else if (ijendQ && (m == (int)contents.size() - 1)) {
                        syls[m]->SetType("repetition repetition-end");
                    }
                    else {
                        syls[m]->SetType("repetition");
                    }
                }
                else {
                    addTextElement(syls[m], contents[m], "", spacer);
                }
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getVerseLabelText --
//

std::string HumdrumInput::getVerseLabelText(hum::HTp token)
{
    if (!token) {
        return "";
    }
    if (!token->isInterpretation()) {
        return "";
    }
    hum::HumRegex hre;
    if (hre.search(token, "^\\*vv?:(.*)", "i")) {
        std::string output = hre.getMatch(1);
        if (hre.search(output, "^\\d+$")) {
            output += ".";
        }
        return output;
    }
    return "";
}

//////////////////////////////
//
// HumdrumInput::splitSyllableBySpaces -- Split a string into pieces
//    according to spaces.  Default value spacer = ' ');
//

void HumdrumInput::splitSyllableBySpaces(vector<std::string> &vtext, char spacer)
{
    if (vtext[0].find(spacer) == std::string::npos) {
        return;
    }
    if (vtext.size() != 1) {
        // invalid size
        return;
    }
    std::string original = vtext[0];
    vtext[0] = "";
    for (int i = 0; i < (int)original.size(); ++i) {
        if (original[i] != spacer) {
            vtext.back().push_back(original[i]);
            continue;
        }
        // new string needs to be made
        vtext.push_back("");
    }
}

//////////////////////////////
//
// HumdrumInput::convertMensuralRhythm --
//

template <class ELEMENT> hum::HumNum HumdrumInput::convertMensuralRhythm(ELEMENT element, hum::HTp token, int subtoken)
{

    std::string tstring;
    if (subtoken < 0) {
        tstring = *token;
        // strip off any leading spaces
        auto first = tstring.find_first_not_of(' ');
        if (first != std::string::npos) {
            tstring = tstring.substr(first);
        }
    }
    else {
        tstring = token->getSubtoken(subtoken);
    }

    std::string vstring = token->getVisualDuration(subtoken);

    // Tuplet durations are not handled below yet.
    // dur is in units of quarter notes.

    hum::HumNum dur;
    hum::HumNum durges;

    if (vstring.empty()) {
        dur = hum::Convert::mensToDurationNoDots(tstring);
        dur /= 4; // duration is now in whole note units;
        dur *= m_tupletscaling;
    }
    else {
        dur = hum::Convert::mensToDurationNoDots(vstring);
        dur /= 4; // duration is now in whole note units;
        dur *= m_tupletscaling;

        durges = hum::Convert::mensToDurationNoDots(tstring);
        durges /= 4; // duration is now in whole note units;
        durges *= m_tupletscaling;
    }

    if (dur.isInteger()) {
        switch (dur.getNumerator()) {
            case 1: element->SetDur(DURATION_semibrevis); break;
            case 2: element->SetDur(DURATION_brevis); break;
            case 4: element->SetDur(DURATION_longa); break;
            case 8: element->SetDur(DURATION_maxima); break;
        }
    }
    else if (dur.getNumerator() == 1) {
        switch (dur.getDenominator()) {
            case 2: element->SetDur(DURATION_minima); break;
            case 4: element->SetDur(DURATION_semiminima); break;
            case 8: element->SetDur(DURATION_fusa); break;
            case 16: element->SetDur(DURATION_semifusa); break;
            case 32: element->SetDur(DURATION_32); break;
            case 64: element->SetDur(DURATION_64); break;
            case 128: element->SetDur(DURATION_128); break;
            case 256: element->SetDur(DURATION_256); break;
            case 512: element->SetDur(DURATION_512); break;
            case 1024: element->SetDur(DURATION_1024); break;
            case 2048: element->SetDur(DURATION_2048); break;
        }
    }

    if (!vstring.empty()) {
        if (durges.isInteger()) {
            switch (durges.getNumerator()) {
                case 1: element->SetDurGes(DURATION_semibrevis); break;
                case 2: element->SetDurGes(DURATION_brevis); break;
                case 4: element->SetDurGes(DURATION_longa); break;
                case 8: element->SetDurGes(DURATION_maxima); break;
            }
        }
        else if (durges.getNumerator() == 1) {
            switch (durges.getDenominator()) {
                case 2: element->SetDurGes(DURATION_minima); break;
                case 4: element->SetDurGes(DURATION_semiminima); break;
                case 8: element->SetDurGes(DURATION_fusa); break;
                case 16: element->SetDurGes(DURATION_semifusa); break;
                case 32: element->SetDurGes(DURATION_32); break;
                case 64: element->SetDurGes(DURATION_64); break;
                case 128: element->SetDurGes(DURATION_128); break;
                case 256: element->SetDurGes(DURATION_256); break;
                case 512: element->SetDurGes(DURATION_512); break;
                case 1024: element->SetDurGes(DURATION_1024); break;
                case 2048: element->SetDurGes(DURATION_2048); break;
            }
        }
        return durges;
    }
    else {
        /*
                if (dur.isInteger()) {
                        switch (dur.getNumerator()) {
                        case 1: element->SetDurGes(DURATION_1); break;
                        case 2: element->SetDurGes(DURATION_breve); break;
                        case 4: element->SetDurGes(DURATION_long); break;
                        case 8: element->SetDurGes(DURATION_maxima); break;
                        }
                }
                else if (dur.getNumerator() == 1) {
                        switch (dur.getDenominator()) {
                        case 2: element->SetDurGes(DURATION_2); break;
                        case 4: element->SetDurGes(DURATION_4); break;
                        case 8: element->SetDurGes(DURATION_8); break;
                        case 16: element->SetDurGes(DURATION_16); break;
                        case 32: element->SetDurGes(DURATION_32); break;
                        case 64: element->SetDurGes(DURATION_64); break;
                        case 128: element->SetDurGes(DURATION_128); break;
                        case 256: element->SetDurGes(DURATION_256); break;
                        case 512: element->SetDurGes(DURATION_512); break;
                        case 1024: element->SetDurGes(DURATION_1024); break;
                        case 2048: element->SetDurGes(DURATION_2048); break;
                        }
                }
        */
    }

    return dur;
}

//////////////////////////////
//
// HumdrumInput::setRhythmFromDuration --
//

template <class ELEMENT> void HumdrumInput::setRhythmFromDuration(ELEMENT element, hum::HumNum duration)
{
    pair<data_DURATION, int> durAndDots = getDurAndDots(duration);
    element->SetDur(durAndDots.first);
    if (durAndDots.second != 0) {
        element->SetDots(durAndDots.second);
    }
}

//////////////////////////////
//
// HumdrumInput::setVisualAndGesturalRhythmFromDuration --
//

template <class ELEMENT>
void HumdrumInput::setVisualAndGesturalRhythmFromDuration(ELEMENT element, hum::HumNum visdur, hum::HumNum gesdur)
{
    pair<data_DURATION, int> visDurAndDots = getDurAndDots(visdur);
    element->SetDur(visDurAndDots.first);
    if (visDurAndDots.second != 0) {
        element->SetDots(visDurAndDots.second);
    }
    pair<data_DURATION, int> gesDurAndDots = getDurAndDots(gesdur);
    if (gesDurAndDots.first != visDurAndDots.first) {
        element->SetDurGes(gesDurAndDots.first);
    }
    if (gesDurAndDots.second != visDurAndDots.second) {
        element->SetDotsGes(gesDurAndDots.second);
    }
}

//////////////////////////////
//
// HumdrumInput::convertRhythm --
//     default value:
//         subtoken = -1;
//         isnote = true
//

template <class ELEMENT> hum::HumNum HumdrumInput::convertRhythm(ELEMENT element, hum::HTp token, int subtoken)
{
    if (token->isMensLike()) {
        return convertMensuralRhythm(element, token, subtoken);
    }

    std::string tstring;
    if (subtoken < 0) {
        tstring = *token;
        // strip off any leading spaces
        auto first = tstring.find_first_not_of(' ');
        if (first != std::string::npos) {
            tstring = tstring.substr(first);
        }
    }
    else {
        tstring = token->getSubtoken(subtoken);
    }

    // Remove grace note information (for generating printed duration)
    bool grace = false;
    if (tstring.find('q') != std::string::npos) {
        grace = true;
        tstring.erase(std::remove(tstring.begin(), tstring.end(), 'q'), tstring.end());
    }

    bool overfillQ = false;
    if (!grace) {
        overfillQ = processOverfillingNotes(token);
    }

    std::string vstring;
    if (overfillQ) {
        vstring = token->getValue("auto", "N", "vis");
    }
    else if (subtoken < 0) {
        vstring = token->getVisualDurationChord();
    }
    else {
        vstring = token->getVisualDuration(subtoken);
    }

    if (!vstring.empty()) {
        int visualdotcount = characterCountInSubtoken(vstring, '.');
        if (visualdotcount > 0) {
            element->SetDots(visualdotcount);
        }
        int gesturaldotcount = characterCountInSubtoken(tstring, '.');
        if (gesturaldotcount != visualdotcount) {
            element->SetDotsGes(gesturaldotcount);
            element->SetDots(visualdotcount);
        }
    }
    else {
        int dotcount = characterCountInSubtoken(tstring, '.');
        if (dotcount > 0) {
            element->SetDots(dotcount);
        }
    }

    // Tuplet durations are not handled below yet.
    // dur is in units of quarter notes.

    hum::HumNum dur;
    hum::HumNum durges;

    if (overfillQ) {
        std::string logicaldur = token->getValue("auto", "MEI", "dur.logical");
        durges = hum::Convert::recipToDurationNoDots(logicaldur);
        durges /= 4; // convert duration to whole-note units
        std::string visualdur = token->getValue("auto", "N", "vis");
        dur = hum::Convert::recipToDurationNoDots(visualdur);
        dur /= 4; // convert duration to whole-note units
        int logicaldurdots = (int)std::count(logicaldur.begin(), logicaldur.end(), '.');
        int visualdurdots = (int)std::count(visualdur.begin(), visualdur.end(), '.');
        if (visualdurdots != logicaldurdots) {
            element->SetDotsGes(logicaldurdots);
        }
        std::string typestring = token->getValue("auto", "MEI", "type");
        if (typestring.empty()) {
            element->SetType("overfill");
        }
        else {
            element->SetType(typestring);
        }
    }
    else if (vstring.empty()) {
        dur = hum::Convert::recipToDurationNoDots(tstring);
        dur /= 4; // convert duration to whole-note units
        if (!grace) {
            dur *= m_tupletscaling;
        }
    }
    else {
        dur = hum::Convert::recipToDurationNoDots(vstring);
        dur /= 4; // convert duration to whole-note units
        if (!grace) {
            dur *= m_tupletscaling;
        }

        durges = hum::Convert::recipToDurationNoDots(tstring);
        durges /= 4; // convert duration to whole-note units
        if (!grace) {
            durges *= m_tupletscaling;
        }
    }

    if ((!grace) && (dur == 0) && element) {
        return 0;
        // duration should be set to "1" to make it look like
        // a stemless note with a quarter note duration to make
        // it a quarter notehead shape (maybe just set the notehead
        // shape to a quarter note).  But cannot set the stem
        // length in this function since non-notes can have
        // durations (rests and spaces), so the calling function
        // has to take character.
    }

    if (dur.isInteger()) {
        switch (dur.getNumerator()) {
            case 1: element->SetDur(DURATION_1); break;
            case 2: element->SetDur(DURATION_breve); break;
            case 4: element->SetDur(DURATION_long); break;
            case 8: element->SetDur(DURATION_maxima); break;
        }
    }
    else if (dur.getNumerator() == 1) {
        switch (dur.getDenominator()) {
            case 2: {
                element->SetDur(DURATION_2);
                // handle stem position tandem insterpretations
                int staff = m_currentstaff;
                int staffindex = staff - 1;
                std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
                if (ss[staffindex].righthalfstem
                    && ((element->GetClassName() == "Note") || (element->GetClassName() == "Chord"))) {
                    m_setrightstem = true;
                }
            } break;
            case 4: element->SetDur(DURATION_4); break;
            case 8: element->SetDur(DURATION_8); break;
            case 16: element->SetDur(DURATION_16); break;
            case 32: element->SetDur(DURATION_32); break;
            case 64: element->SetDur(DURATION_64); break;
            case 128: element->SetDur(DURATION_128); break;
            case 256: element->SetDur(DURATION_256); break;
            case 512: element->SetDur(DURATION_512); break;
            case 1024: element->SetDur(DURATION_1024); break;
            case 2048: element->SetDur(DURATION_2048); break;
        }
    }

    if (!vstring.empty()) {
        if (durges.isInteger()) {
            switch (durges.getNumerator()) {
                case 1: element->SetDurGes(DURATION_1); break;
                case 2: element->SetDurGes(DURATION_breve); break;
                case 4: element->SetDurGes(DURATION_long); break;
                case 8: element->SetDurGes(DURATION_maxima); break;
            }
        }
        else if (durges.getNumerator() == 1) {
            switch (durges.getDenominator()) {
                case 2: element->SetDurGes(DURATION_2); break;
                case 4: element->SetDurGes(DURATION_4); break;
                case 8: element->SetDurGes(DURATION_8); break;
                case 16: element->SetDurGes(DURATION_16); break;
                case 32: element->SetDurGes(DURATION_32); break;
                case 64: element->SetDurGes(DURATION_64); break;
                case 128: element->SetDurGes(DURATION_128); break;
                case 256: element->SetDurGes(DURATION_256); break;
                case 512: element->SetDurGes(DURATION_512); break;
                case 1024: element->SetDurGes(DURATION_1024); break;
                case 2048: element->SetDurGes(DURATION_2048); break;
            }
        }
        hum::HumNum newdur = hum::Convert::recipToDuration(vstring);
        // See note eDcRfV
        setRhythmFromDuration(element, newdur);
        return newdur;
    }

    return dur;
}

//////////////////////////////
//
// HumdrumInput::addBreath -- Add floating breath for note/chord.
//     default value: parent = NULL
//

void HumdrumInput::addBreath(hum::HTp token, Object *parent)
{
    int layer = m_currentlayer;
    int staff = getNoteStaff(token, m_currentstaff);

    if (token->find(",") == std::string::npos) {
        return;
    }

    if ((token->find("yy") == std::string::npos) && (token->find(",y") == std::string::npos)) {
        Breath *breath = new Breath();
        appendElement(m_measure, breath);
        setStaff(breath, staff);

        if (parent && (token->find("q") != std::string::npos)) {
            // grace notes cannot be addressed with @tstamp, so
            // have to use @startid.  Maybe allow @tstamp, since
            // @startid will probably shift to the correct grace note
            // position.
            std::string id = "#" + parent->GetID();
            breath->SetStartid(id);
        }
        else if (!token->empty() && (token->at(0) == '=')) {
            // barline breath
            if (parent) {
                std::string id = "#" + parent->GetID();
                breath->SetStartid(id);
            }
            else {
                hum::HumNum tstamp = getMeasureEndTstamp(staff - 1);
                breath->SetTstamp(tstamp.getFloat());
            }
        }
        else {
            hum::HumNum start = getMeasureTstamp(token, staff - 1);
            hum::HumNum dur = token->getDuration() * getMeasureFactor(staff - 1);
            hum::HumNum tstamp = start + ((dur * 4) / 5);
            breath->SetTstamp(tstamp.getFloat());
        }
        setLocationId(breath, token);

        int direction = getDirection(*token, ",");
        if (direction < 0) {
            setPlaceRelStaff(breath, "below", false);
        }
        else if (direction > 0) {
            setPlaceRelStaff(breath, "above", false);
        }
        else if (layer == 1) {
            setPlaceRelStaff(breath, "above", false);
        }
        else if (layer == 2) {
            setPlaceRelStaff(breath, "below", false);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getStaffAdjustment -- +1 if all notes are staff below
//     and -1 if all notes are staff above.
//

int HumdrumInput::getStaffAdjustment(hum::HTp token)
{
    hum::HumRegex hre;
    // Check for notes in chords that are all on a different staff.  If so,
    // then move the fermata to the different staff.
    std::vector<std::string> subtokens;
    int scount = token->getSubtokenCount();
    for (int i = 0; i < scount; ++i) {
        subtokens.emplace_back(token->getSubtoken(i));
    }
    int allabove = true;
    int allbelow = true;
    std::string upquery = "[A-Ga-gr][#n-]*[xXyY]*";
    upquery += m_signifiers.above;
    std::string downquery = "[A-Ga-gr][#n-]*[xXyY]*";
    downquery += m_signifiers.below;
    if (m_signifiers.above) {
        for (int i = 0; i < scount; ++i) {
            if (!hre.search(subtokens[i], upquery)) {
                allabove = false;
                break;
            }
        }
    }
    else {
        allabove = false;
    }
    if (m_signifiers.below && !allabove) {
        for (int i = 0; i < scount; ++i) {
            if (!hre.search(subtokens[i], downquery)) {
                allbelow = false;
                break;
            }
        }
    }
    else {
        allbelow = false;
    }
    int staffadj = 0;
    if (allabove) {
        staffadj = -1; // -1 means up
    }
    else if (allbelow) {
        staffadj = +1; // +1 means down
    }
    return staffadj;
}

//////////////////////////////
//
// HumdrumInput::addFermata -- Add floating fermatas for note/chord.
//     default value: parent = NULL
//

template <class ELEMENT> void HumdrumInput::addFermata(ELEMENT *rest, const std::string &tstring)
{

    if ((tstring.find("yy") == std::string::npos) && (tstring.find(";y") == std::string::npos)) {
        // Inform the document that there are analytic fermatas in the data
        // (@fermata as opposed to (or in addition to <fermata>).
        // This allows verovio to render the fermata on the rest in the
        // SVG conversion.  Input can be Rest or MRest.
        m_doc->SetMarkup(MARKUP_ANALYTICAL_FERMATA);

        int layer = m_currentlayer;

        int direction = getDirection(tstring, ";");
        if (direction < 0) {
            rest->SetFermata(STAFFREL_basic_below);
        }
        else if (direction > 0) {
            rest->SetFermata(STAFFREL_basic_above);
        }
        else if (layer == 1) {
            rest->SetFermata(STAFFREL_basic_above);
        }
        else if (layer == 2) {
            rest->SetFermata(STAFFREL_basic_below);
        }
        else {
            // who knows, maybe check the stem direction or see
            // if another note/rest in a different layer already
            // has a fermata (so you would not want to overwrite them).
            rest->SetFermata(STAFFREL_basic_above);
        }
    }
}

void HumdrumInput::addFermata(hum::HTp token, Object *parent)
{
    int layer = m_currentlayer;
    int staff = m_currentstaff;

    if (token->find(";") == std::string::npos) {
        return;
    }

    int staffadj = getStaffAdjustment(token);

    if ((token->find("yy") == std::string::npos) && (token->find(";y") == std::string::npos)) {
        Fermata *fermata = new Fermata();
        appendElement(m_measure, fermata);
        setStaff(fermata, staff + staffadj);

        Fermata *fermata2 = NULL;
        if (token->find(";;") != std::string::npos) {
            fermata2 = new Fermata();
            appendElement(m_measure, fermata2);
            setStaff(fermata2, staff + staffadj);
        }

        if (parent && (token->find("q") != std::string::npos)) {
            // grace notes cannot be addressed with @tstamp, so
            // have to use @startid.  Maybe allow @tstamp, since
            // @startid will probably shift to the correct grace note
            // position.
            std::string id = "#" + parent->GetID();
            fermata->SetStartid(id);
            if (fermata2) {
                fermata2->SetStartid(id);
            }
        }
        else if (!token->empty() && (token->at(0) == '=')) {
            // barline fermata
            if (parent) {
                std::string id = "#" + parent->GetID();
                fermata->SetStartid(id);
                if (fermata2) {
                    fermata2->SetStartid(id);
                }
            }
            else {
                hum::HumNum tstamp = getMeasureEndTstamp(staff - 1);
                fermata->SetTstamp(tstamp.getFloat());
                if (fermata2) {
                    fermata2->SetTstamp(tstamp.getFloat());
                }
            }
        }
        else {
            hum::HumNum tstamp = getMeasureTstamp(token, staff - 1);
            if (parent) {
                std::string id = "#" + parent->GetID();
                fermata->SetStartid(id);
                if (fermata2) {
                    fermata2->SetStartid(id);
                }
            }
            else {
                fermata->SetTstamp(tstamp.getFloat());
                if (fermata2) {
                    fermata2->SetTstamp(tstamp.getFloat());
                }
            }
        }
        // if a barline, then can have two fermatas
        if (fermata2) {
            setLocationIdNSuffix(fermata, token, 1);
            setLocationIdNSuffix(fermata2, token, 2);
        }
        else {
            setLocationId(fermata, token);
        }

        if (fermata2) {
            setPlaceRelStaff(fermata, "above", false);
            setPlaceRelStaff(fermata2, "below", false);
            return;
        }

        int direction = getDirection(*token, ";");
        if (direction < 0) {
            setPlaceRelStaff(fermata, "below", false);
        }
        else if (direction > 0) {
            setPlaceRelStaff(fermata, "above", false);
        }
        else if (layer == 1) {
            setPlaceRelStaff(fermata, "above", false);
        }
        else if (layer == 2) {
            setPlaceRelStaff(fermata, "below", false);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::addArpeggio --
//   : = arpeggio which may cross between layers on a staff.
//   :: = arpeggio which crosses staves on a single system.
//

void HumdrumInput::addArpeggio(Object *object, hum::HTp token)
{

    bool systemQ = false;
    bool staffQ = false;
    std::vector<hum::HTp> arpTokens;
    if (token->find("::") != std::string::npos) {
        if (!isLeftmostSystemArpeggio(token)) {
            return;
        }
        systemQ = true;
        arpTokens = getSystemArpeggioTokens(token);
    }
    else if (token->find(":") != std::string::npos) {
        if (!isLeftmostStaffArpeggio(token)) {
            return;
        }
        staffQ = true;
        arpTokens = getStaffArpeggioTokens(token);
    }
    else {
        return; // no arpeggio on this note/chord
    }

    // int layer = m_currentlayer;
    // int staff = m_currentstaff;

    if (systemQ || staffQ) {
        Arpeg *arpeg = new Arpeg();
        appendElement(m_measure, arpeg);
        // no staff attachment, or list both endpoint staves or all staves involved?
        setLocationId(arpeg, token);
        std::string firstid = object->GetID();
        arpeg->AddRef("#" + firstid);

        for (auto it = arpTokens.begin(); it != arpTokens.end(); ++it) {
            hum::HTp earp = *it;
            std::string nextid;
            if (earp->find(" ") != std::string::npos) {
                nextid = getLocationId("chord", earp);
            }
            else {
                nextid = getLocationId("note", earp);
            }
            arpeg->AddRef("#" + nextid);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getStaffArpeggioTokens -- Assuming a single contiguous
//     arpeggio across all layers from first to last marker.
//

std::vector<hum::HTp> HumdrumInput::getStaffArpeggioTokens(hum::HTp token)
{
    std::vector<hum::HTp> output;
    int track = token->getTrack();
    token = token->getNextFieldToken();
    int ntrack = 0;
    if (token) {
        ntrack = token->getTrack();
    }
    while (token != NULL) {
        if (ntrack != track) {
            break;
        }
        if (!token->isKernLike()) {
            token = token->getNextFieldToken();
            continue;
        }
        if ((token->find(":") != std::string::npos) && (token->find("::") == std::string::npos)) {
            output.push_back(token);
        }
        token = token->getNextFieldToken();
        if (token) {
            ntrack = token->getTrack();
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::leftmostStaffArpeggio --
//

bool HumdrumInput::isLeftmostStaffArpeggio(hum::HTp token)
{
    int track = token->getTrack();
    token = token->getPreviousFieldToken();
    int ntrack = 0;
    if (token) {
        ntrack = token->getTrack();
    }
    while (token != NULL) {
        if (track != ntrack) {
            break;
        }
        if (!token->isKernLike()) {
            token = token->getPreviousFieldToken();
            if (token) {
                ntrack = token->getTrack();
            }
            continue;
        }
        if (token->find(":") != std::string::npos) {
            return false;
        }
        token = token->getPreviousFieldToken();
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::getSystemArpeggioTokens -- Assuming a single contiguous
//     arpeggio across all staves from first to last marker.  Will probably have
//     to adjust for layers (which are ordered reversed compared to staves).
//

std::vector<hum::HTp> HumdrumInput::getSystemArpeggioTokens(hum::HTp token)
{
    std::vector<hum::HTp> output;
    token = token->getNextFieldToken();
    while (token != NULL) {
        if (!token->isKernLike()) {
            token = token->getNextFieldToken();
            continue;
        }
        if (token->find("::") != std::string::npos) {
            output.push_back(token);
        }
        token = token->getNextFieldToken();
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::isLowestSystemArpeggio -- Not checking for contiguous staves
//  (i.e., only one cross-staff arpeggio is allowed at a time for now).  Will
//  probably have to adjust for layers (which are ordered reverse of staves).
//  maybe limit to +1/-1 one staff, but there will still be complications
//  with adjacent grand staff parts.
//

bool HumdrumInput::isLeftmostSystemArpeggio(hum::HTp token)
{
    token = token->getPreviousFieldToken();
    while (token != NULL) {
        if (!token->isKernLike()) {
            token = token->getPreviousFieldToken();
            continue;
        }
        if (token->find("::") != std::string::npos) {
            return false;
        }
        token = token->getPreviousFieldToken();
    }
    return true;
}

//////////////////////////////
//
// HumdrumInput::addOrnaments --
//   M  = mordent, major second for top interval
//   m  = mordent, minor second for top interval
//   W  = inverted mordent, major second for top interval
//   w  = inverted mordent, minor second for top interval
//   T  = trill, major second
//   t  = trill, minor second
//   TT  = trill, major second with wavy line after it
//   tt  = trill, minor second with wavy line after it
//   TTT = trill, continuing a major second with wavy line after it
//   ttt = trill, continuing a minor second with wavy line after it
//
//   S[Ss]?[Ss]? = turn
//   $[Ss]?[Ss]? = inverted turn
//
//   These are not used anymore:
//   SS = turn centered between two notes.  Now a turn starting with
//        S or $ will be centered.
//   $$ = inverted turn centered between two notes.
//   To uncenter a turn (attach to a note), prefix the turn
//   with lower-case s, such as sSSS, each character meaning:
//       s = do not center turn
//       S = regular turn
//       S = Major second interval above turn note
//       S = Major second interval below turn note
//

void HumdrumInput::addOrnaments(Object *object, hum::HTp token)
{
    std::vector<string> subtoks = token->getSubtokens();

    for (int t = 0; t < (int)subtoks.size(); t++) {
        std::vector<bool> chartable(128, false);
        for (int i = 0; i < (int)subtoks.at(t).size(); ++i) {
            int intch = (unsigned char)subtoks.at(t).at(i);
            if (intch < 0 || intch > 127) {
                continue;
            }
            chartable[intch] = true;
        }

        if (chartable['T'] || chartable['t']) {
            addTrill(object, token);
        }
        if (chartable[';']) {
            addFermata(token, object);
        }
        if (chartable[',']) {
            addBreath(token, object);
        }
        if (chartable['W'] || chartable['w'] || chartable['M'] || chartable['m']) {
            addMordent(object, token);
        }
        if (chartable['s'] || chartable['S'] || chartable['$']) {
            addTurn(token, subtoks.at(t), subtoks.size() > 1 ? t : -1);
        }
    }

    // addOrnamentMarkers(token);
}

//////////////////////////////
//
// HumdrumInput::addTurn -- Add turn for note.
//  only one of these four possibilities:
//      S([Ss][Ss])?  = delayed turn
//      sS([Ss][Ss])? = undelayed turn
//      $([Ss][Ss])?  = delayed inverted turn
//      s$([Ss][Ss])? = undelayed inverted turn
//
//  Not used anymore:
//      SS = turn, centered between two notes
//      $$ = inverted turn, centered between two notes
//
//  Layout parameters:
//      LO:TURN:facc[=true] = flip upper and lower accidentals
//      LO:TURN:uacc=[acc]  = upper [visible] accidental (or lower visual one if flip is active)
//      LO:TURN:lacc=[acc]  = lower [visible] accidental (or upper visual one if flip is active)
//             [ul]acc = "none" = force the accidental not to show
//             [ul]acc = "true" = force the accidental not to show ("LO:TURN:[ul]acc" hide an accidental)
//
// Deal with cases where the accidental should be hidden but different from sounding accidental.  This
// can be done when MEI allows @accidlower.ges and @accidupper.ges.
//
// noteIndex == -1 means the note is not in a chord; otherwise, the noteIndex is the nth note in
// a chord (from left to right in a token).
//

void HumdrumInput::addTurn(hum::HTp token, const string &tok, int noteIndex)
{
    int turnstart = -1;
    int turnend = -1;

    for (int i = 0; i < (int)tok.size(); ++i) {
        if ((tok[i] == 's') || (tok[i] == 'S') || (tok[i] == '$')) {
            turnstart = i;
            turnend = i;
            for (int j = i + 1; j < (int)tok.size(); j++) {
                if (!((tok[j] == 's') || (tok[j] == 'S') || (tok[j] == '$'))) {
                    turnend = j - 1;
                    break;
                }
                else {
                    turnend = j;
                }
            }
            break;
        }
    }

    if (turnstart == turnend) {
        return;
    }
    std::string turnstr = tok.substr(turnstart, turnend - turnstart + 1);

    if (turnstr.empty()) {
        return;
    }

    bool delayedQ = turnstr.at(0) == 's' ? false : true;
    if ((!delayedQ) && (turnstr.size() == 1)) {
        // not a valid turn indication
        return;
    }

    bool invertedQ = false;
    if ((turnstr.size() > 1) && (((!delayedQ) && turnstr.at(1) == '$') || (turnstr.at(0) == '$'))) {
        invertedQ = true;
    }

    // int layer = m_currentlayer; // maybe place below if in layer 2
    int staff = getNoteStaff(token, m_currentstaff);

    Turn *turn = new Turn();
    appendElement(m_measure, turn);
    setStaff(turn, staff);

    if (delayedQ) {
        turn->SetDelayed(BOOLEAN_true);
    }

    std::string noteid = getLocationId("note", token, noteIndex);
    turn->SetStartid("#" + noteid);

    turn->SetForm(invertedQ ? turnLog_FORM_lower : turnLog_FORM_lower);

    if (m_signifiers.above) {
        if (turnend < (int)token->size() - 1) {
            if ((*token)[turnend + 1] == m_signifiers.above) {
                setPlaceRelStaff(turn, "above", true);
            }
        }
    }
    if (m_signifiers.below) {
        if (turnend < (int)token->size() - 1) {
            if ((*token)[turnend + 1] == m_signifiers.below) {
                setPlaceRelStaff(turn, "below", true);
            }
        }
    }

    int subtok = noteIndex;
    int tokindex = subtok;
    if (subtok < 0) {
        tokindex = 0;
    }

    // Check for automatic upper and lower accidental on turn:
    std::string loweraccid;
    std::string upperaccid;
    loweraccid = token->getValue("auto", to_string(tokindex), "turnLowerAccidental");
    upperaccid = token->getValue("auto", to_string(tokindex), "turnUpperAccidental");
    if (!loweraccid.empty()) {
        if (loweraccid == "1") {
            loweraccid = "#";
        }
        if (loweraccid == "-1") {
            loweraccid = "-";
        }
        if (loweraccid == "0") {
            loweraccid = "n";
        }
        if (loweraccid == "3") {
            loweraccid = "#x";
        }
        if (loweraccid == "-3") {
            loweraccid = "---";
        }
        if (loweraccid == "2") {
            loweraccid = "x";
        }
        else if (loweraccid == "-2") {
            loweraccid = "--";
        }
    }
    if (!upperaccid.empty()) {
        if (upperaccid == "1") {
            upperaccid = "#";
        }
        if (upperaccid == "-1") {
            upperaccid = "-";
        }
        if (upperaccid == "0") {
            upperaccid = "n";
        }
        if (upperaccid == "3") {
            upperaccid = "#x";
        }
        if (upperaccid == "-3") {
            upperaccid = "---";
        }
        if (upperaccid == "2") {
            upperaccid = "x";
        }
        else if (upperaccid == "-2") {
            upperaccid = "--";
        }
    }

    // Check for LO:TURN forced visual accidentals:
    std::string lacctext = token->getLayoutParameter("TURN", "lacc");
    std::string uacctext = token->getLayoutParameter("TURN", "uacc");

    if (!lacctext.empty()) {
        if (lacctext == "none") {
            loweraccid = "none";
        }
        if (lacctext == "true") {
            loweraccid = "none";
        }
        else {
            loweraccid = lacctext;
        }
    }
    if (!uacctext.empty()) {
        if (uacctext == "false") {
            upperaccid = "none";
        }
        if (uacctext == "true") {
            upperaccid = "none";
        }
        else {
            upperaccid = uacctext;
        }
    }

    // Check to see if accidentals need to be flipped:
    std::string text = token->getLayoutParameter("TURN", "facc");
    if (text == "true") {
        std::string tval = loweraccid;
        loweraccid = upperaccid;
        upperaccid = tval;
    }

    if (!loweraccid.empty()) {
        setWrittenAccidentalLower(turn, loweraccid);
    }
    if (!upperaccid.empty()) {
        setWrittenAccidentalUpper(turn, upperaccid);
    }
}

//////////////////////////////
//
// HumdrumInput::setWrittenAccidentalUpper -- An empty input string will be ignored, use "none" to force
//     no upper accidental.
//

template <class ELEMENT> void HumdrumInput::setWrittenAccidentalUpper(ELEMENT element, const std::string &value)
{
    if (value == "none") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_NONE);
    }
    else if (value == "#") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_s);
    }
    else if (value == "-") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_f);
    }
    else if (value == "n") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_n);
    }
    else if (value == "n-") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_nf);
    }
    else if (value == "n#") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_ns);
    }
    else if (value == "--") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_ff);
    }
    else if (value == "##") {
        element->SetAccidupper(ACCIDENTAL_WRITTEN_x);
    }
    else {
        // other accidentals are possible to add here; otherwise,
        // the invalid accidental will be ignored (not replaced
        // with an empty accidental, for example).
    }
}

//////////////////////////////
//
// HumdrumInput::setWrittenAccidentalLower -- An empty input string will be ignored, use "none" to force
//     no upper accidental.
//

template <class ELEMENT> void HumdrumInput::setWrittenAccidentalLower(ELEMENT element, const std::string &value)
{
    if (value == "none") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_NONE);
    }
    else if (value == "#") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_s);
    }
    else if (value == "-") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_f);
    }
    else if (value == "n") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_n);
    }
    else if (value == "n-") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_nf);
    }
    else if (value == "n#") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_ns);
    }
    else if (value == "--") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_ff);
    }
    else if (value == "##") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_x);
    }
    else if (value == "x") {
        element->SetAccidlower(ACCIDENTAL_WRITTEN_x);
    }
    else {
        // other accidentals are possible to add here; otherwise,
        // the invalid accidental will be ignored (not replaced
        // with an empty accidental, for example).
    }
}

//////////////////////////////
//
// HumdrumInput::addMordent -- Add mordent for note.
//      M  = upper mordent, major second interval
//      MM = double upper mordent, major second interval
//      m  = upper mordent, minor second interval
//      mm = double upper mordent, minor second interval
//      W  = lower mordent, major second interval
//      WW = double lower mordent, major second interval
//      w  = lower mordent, minor second interval
//      ww = double lower mordent, minor second interval
//  also:
//      Mm  = upper mordent with unknown interval
//      MMm = double upper mordent with unknown interval
//      Ww  = lower mordent with unknown interval
//      WWw = lower mordent with unknown interval
//
//

void HumdrumInput::addMordent(Object *linked, hum::HTp token)
{
    std::vector<std::string> subtoks = token->getSubtokens();
    if (subtoks.empty()) {
        return;
    }

    int subtrack = token->getSubtrack();
    std::vector<int> mindex; // index of subtokens with mordent
    std::vector<std::string> mstring; // list of mordent string and any placements
    std::vector<int> mpitch; // pitch of mordent note

    hum::HumRegex hre;
    std::string query = "(";
    query += "[wWmM]+";
    query += "[y";
    if (m_signifiers.above) {
        query += m_signifiers.above;
    }
    if (m_signifiers.below) {
        query += m_signifiers.below;
    }
    query += "]*";
    query += ")";

    for (int i = 0; i < (int)subtoks.size(); ++i) {
        if (subtoks[i].find('r') != std::string::npos) {
            continue;
        }
        if (!hre.search(subtoks[i], query)) {
            continue;
        }
        std::string match = hre.getMatch(1);
        if (match.find("y") != std::string::npos) {
            // Hidden mordent, so suppress from conversion.
            continue;
        }
        mindex.push_back(i);
        mstring.push_back(match);
        mpitch.push_back(hum::Convert::kernToBase40(subtoks[i]));
    }

    if (mindex.empty()) {
        return;
    }

    int highest = mpitch.at(0);
    int lowest = mpitch.at(0);
    for (int i = 1; i < (int)mpitch.size(); ++i) {
        if (highest < mpitch[i]) {
            highest = mpitch[i];
        }
        if (lowest > mpitch[i]) {
            lowest = mpitch[i];
        }
    }

    std::vector<int> mplace(mindex.size(), 0);
    if (subtrack) {
        if (subtrack % 2) {
            // force above for first/third/fifth/etc. layers:
            std::fill(mplace.begin(), mplace.end(), +1);
        }
        else {
            // force below for second/fourth/etc. layers:
            std::fill(mplace.begin(), mplace.end(), -1);
        }
    }
    else {
        // Single voice so put above if a single mordent,
        // but place a second mordent below if a dyad.
        if (mindex.size() == 1) {
            mplace.at(0) = +1;
        }
        else if (mindex.size() == 2) {
            if (highest == mpitch.at(0)) {
                mplace.at(0) = +1;
                mplace.at(1) = -1;
            }
            else {
                mplace.at(0) = -1;
                mplace.at(1) = +1;
            }
        }
        else {
            // If there are three or more mordents in a chord, place them
            // all above the staff, and if any should be placed below, then
            // it will have to be manually specified.
            std::fill(mplace.begin(), mplace.end(), +1);
        }
    }

    int staff = getNoteStaff(token, m_currentstaff);

    for (int i = 0; i < (int)mindex.size(); ++i) {
        if (mstring.at(i).empty()) {
            continue;
        }
        // Set any explicit placement of the mordent:
        int direction = mplace.at(i);
        if (m_signifiers.above) {
            if (mstring.at(i).find(m_signifiers.above) != std::string::npos) {
                direction = +1;
            }
        }
        if (m_signifiers.below) {
            if (mstring.at(i).find(m_signifiers.below) != std::string::npos) {
                direction = -1;
            }
        }

        bool lowerQ = false;
        if ((mstring[i][0] == 'w') || (mstring[i][0] == 'W')) {
            lowerQ = true;
        }

        Mordent *mordent = new Mordent();
        appendElement(m_measure, mordent);
        setStaff(mordent, staff);

        int subtok = mindex.at(i);
        if (mindex.size() == 1) {
            subtok = -1;
        }
        mordent->SetStartid("#" + linked->GetID());
        setLocationId(mordent, token, subtok);

        if (lowerQ) {
            mordent->SetForm(mordentLog_FORM_lower);
        }
        else {
            mordent->SetForm(mordentLog_FORM_upper);
        }

        if (direction < 0) {
            setPlaceRelStaff(mordent, "below", false);
        }
        else if (direction > 0) {
            setPlaceRelStaff(mordent, "above", false);
        }

        int tokindex = subtok;
        if (tokindex < 0) {
            tokindex = 0;
        }

        if ((mstring.at(i).find('w') != std::string::npos) || (mstring.at(i).find('W') != std::string::npos)) {
            // lower mordent
            std::string accid = token->getValue("auto", to_string(tokindex), "mordentLowerAccidental");
            bool hasaccid = accid.empty() ? false : true;
            int accidval = 0;
            if (hasaccid) {
                accidval = stoi(accid);
                switch (accidval) {
                    case -1: mordent->SetAccidlower(ACCIDENTAL_WRITTEN_f); break;
                    case 0: mordent->SetAccidlower(ACCIDENTAL_WRITTEN_n); break;
                    case +1: mordent->SetAccidlower(ACCIDENTAL_WRITTEN_s); break;
                    case -2: mordent->SetAccidlower(ACCIDENTAL_WRITTEN_ff); break;
                    case +2: mordent->SetAccidlower(ACCIDENTAL_WRITTEN_x); break;
                }
            }
        }
        else {
            // upper mordent
            std::string accid = token->getValue("auto", to_string(tokindex), "mordentUpperAccidental");
            bool hasaccid = accid.empty() ? false : true;
            int accidval = 0;
            if (hasaccid) {
                accidval = stoi(accid);
                switch (accidval) {
                    case -1: mordent->SetAccidupper(ACCIDENTAL_WRITTEN_f); break;
                    case 0: mordent->SetAccidupper(ACCIDENTAL_WRITTEN_n); break;
                    case +1: mordent->SetAccidupper(ACCIDENTAL_WRITTEN_s); break;
                    case -2: mordent->SetAccidupper(ACCIDENTAL_WRITTEN_ff); break;
                    case +2: mordent->SetAccidupper(ACCIDENTAL_WRITTEN_x); break;
                }
            }
        }

        if (hre.search(mstring.at(i), "MM|WW", "i")) {
            mordent->SetLong(BOOLEAN_true);
        }

        // Set an explicit visual accidental for the mordent.
        // Maybe in the future allow for lacc and uacc to place the accidental.
        // Also deal multiple mordents in a chord later.
        std::string acctext = token->getLayoutParameter("MOR", "acc");
        if ((!acctext.empty()) && acctext != "true") {
            if (acctext == "false") {
                acctext = "none";
            }
            if (lowerQ) {
                setWrittenAccidentalLower(mordent, acctext);
            }
            else {
                setWrittenAccidentalUpper(mordent, acctext);
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::getNoteStaff -- Adjust the staff based on above/below
//   modifier on notes.
//

int HumdrumInput::getNoteStaff(hum::HTp token, int homestaff)
{
    hum::HumRegex hre;
    if (m_signifiers.above) {
        std::string sstring = "[a-g]+[-#n]*";
        sstring += m_signifiers.above;
        if (hre.search(token, sstring)) {
            return homestaff - 1;
        }
    }
    if (m_signifiers.below) {
        std::string sstring = "[a-g]+[-#n]*";
        sstring += m_signifiers.below;
        if (hre.search(token, sstring)) {
            return homestaff + 1;
        }
    }

    return homestaff;
}

//////////////////////////////
//
// HumdrumInput::addTrill -- Add trill for note.
//

void HumdrumInput::addTrill(Object *linked, hum::HTp token)
{
    int subtok = 0;
    size_t tpos = std::string::npos;
    for (int i = 0; i < (int)token->size(); ++i) {
        if (token->at(i) == ' ') {
            subtok++;
            continue;
        }
        if ((token->at(i) == 't') || (token->at(i) == 'T')) {
            tpos = i;
            if (i < (int)token->size() - 1) {
                // deal with TT or tt for trills with wavy lines
                if ((token->at(i + 1) == 't') || (token->at(i + 1) == 'T')) {
                    tpos++;
                }
            }
            break;
        }
    }
    if (tpos == std::string::npos) {
        // no trill on a note
        return;
    }

    if (token->find("TTT") != std::string::npos) {
        // continuation trill, so don't start a new one
        return;
    }
    if (token->find("ttt") != std::string::npos) {
        // continuation trill, so don't start a new one
        return;
    }

    if ((subtok == 0) && token->find(" ") == std::string::npos) {
        subtok = -1;
    }

    // int layer = m_currentlayer; // maybe place below if in layer 2
    int staff = getNoteStaff(token, m_currentstaff);

    Trill *trill = new Trill();
    appendElement(m_measure, trill);
    setStaff(trill, staff);

    int staffindex = m_currentstaff - 1;

    int layer = m_currentlayer;
    if (layer == 2) {
        // Force the trill below the staff by default:
        setPlaceRelStaff(trill, "below", false);
    }

    trill->SetStartid("#" + linked->GetID());
    // Setting trill@tstamp:
    // hum::HumNum tstamp = getMeasureTstamp(token, staffindex);
    // trill->SetTstamp(tstamp.getFloat());

    setLocationId(trill, token, subtok);

    if (m_signifiers.above) {
        if (tpos < token->size() - 1) {
            if ((*token)[tpos + 1] == m_signifiers.above) {
                setPlaceRelStaff(trill, "above", true);
            }
        }
    }
    if (m_signifiers.below) {
        if (tpos < token->size() - 1) {
            if ((*token)[tpos + 1] == m_signifiers.below) {
                setPlaceRelStaff(trill, "below", true);
            }
        }
    }

    int tokindex = subtok;
    if (tokindex < 0) {
        tokindex = 0;
    }
    std::string accid = token->getValue("auto", to_string(tokindex), "trillAccidental");
    bool hasaccid = accid.empty() ? false : true;
    int accidval = 0;
    if (hasaccid) {
        accidval = stoi(accid);
        switch (accidval) {
            case -1: trill->SetAccidupper(ACCIDENTAL_WRITTEN_f); break;
            case 0: trill->SetAccidupper(ACCIDENTAL_WRITTEN_n); break;
            case 1: trill->SetAccidupper(ACCIDENTAL_WRITTEN_s); break;
            case -2: trill->SetAccidupper(ACCIDENTAL_WRITTEN_ff); break;
            case 2: trill->SetAccidupper(ACCIDENTAL_WRITTEN_x); break;
        }
    }

    // replace the trill accidental if different in layout parameters, such as:
    //    !LO:TR:acc=##
    // for a double sharp, or
    //    !LO:TR:acc=none
    // for no accidental
    int lcount = token->getLinkedParameterSetCount();
    bool foundQ = false;
    std::string value;
    for (int p = 0; p < lcount; ++p) {
        hum::HumParamSet *hps = token->getLinkedParameterSet(p);
        if (hps == NULL) {
            continue;
        }
        if (hps->getNamespace1() != "LO") {
            continue;
        }
        if (hps->getNamespace2() != "TR") {
            continue;
        }
        for (int q = 0; q < hps->getCount(); ++q) {
            std::string key = hps->getParameterName(q);
            if (key == "acc") {
                value = hps->getParameterValue(q);
                foundQ = true;
                break;
            }
        }
    }

    if (foundQ) {
        if (value == "none") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_NONE);
        }
        else if (value == "#") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_s);
        }
        else if (value == "-") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_f);
        }
        else if (value == "n") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_n);
        }
        else if (value == "--") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_ff);
        }
        else if (value == "##") {
            trill->SetAccidupper(ACCIDENTAL_WRITTEN_x);
        }
        else {
            // other trills are possible to add here; otherwise,
            // the invalid trill will be ignored (not replaced
            // with an empty accidental, for example).
        }
    }

    if ((token->find("TT") == std::string::npos) && (token->find("tt") == std::string::npos)) {
        // no line extension needed fro the trill
        return;
    }

    // Find the ending note after the trill line.  Multiple trill line extensions for chord notes
    // are not handled by this algorithm, but these should be rare in notation.  Trills that cross
    // barlines require @tstamp2 rather than @endid to display (possibly a bug in verovio).
    hum::HTp endtok = token->getNextToken();
    hum::HTp lasttok = token;
    hum::HTp lastnote = token;
    hum::HTp lastnoteorbar = token;
    hum::HTp bartok = NULL;
    int barlinecount = 0;
    bool foundbarline = false;
    bool nextnoteafterbarline = false;
    while (endtok) {
        if (endtok->isBarline()) {
            foundbarline = true;
            nextnoteafterbarline = true;
            bartok = endtok;
            barlinecount++;
            lastnoteorbar = endtok;
        }
        if (!endtok->isData()) {
            endtok = endtok->getNextToken();
            nextnoteafterbarline = false;
            continue;
        }
        if (endtok->isNull()) {
            endtok = endtok->getNextToken();
            continue;
        }
        if (endtok->isGrace()) {
            // check to see if the next non-grace note/rest has a TTT or ttt on it.
            // if so, then do not terminate the trill extension line at this
            // grace notes.
            hum::HTp ntok = endtok->getNextToken();
            while (ntok) {
                if (ntok->isBarline()) {
                    lastnoteorbar = ntok;
                }
                if (!ntok->isData()) {
                    ntok = ntok->getNextToken();
                    continue;
                }
                if (ntok->isGrace()) {
                    ntok = ntok->getNextToken();
                    continue;
                }
                lastnoteorbar = ntok;
                lastnote = ntok;
                // at this point ntok is a durational note or rest
                if ((ntok->find("TTT") == std::string::npos) && (ntok->find("ttt") == std::string::npos)) {
                    break;
                }
                ntok = ntok->getNextToken();
            }
        }
        lastnoteorbar = endtok;
        lastnote = endtok;
        if ((endtok->find("TTT") == std::string::npos) && (endtok->find("ttt") == std::string::npos)) {
            break;
        }
        lasttok = endtok;
        endtok = endtok->getNextToken();
    }

    if ((!endtok) && lastnoteorbar && lastnoteorbar->isBarline()) {
        // reached the end of the music, so calculate the duration
        // of the trill extension line from the position of the last
        // barline.
        hum::HumNum tstamp2 = getMeasureTstampPlusDur(bartok, staffindex);
        std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
        hum::HumNum mfactor = ss[staffindex].meter_bottom / 4;
        tstamp2 += mfactor;
        int measures = getMeasureDifference(token, bartok);
        measures--;
        std::pair<int, double> ts2(measures, tstamp2.getFloat());
        trill->SetTstamp2(ts2);
        return;
    }

    if ((!endtok) && lastnoteorbar && lastnoteorbar->isData()) {
        // reached the end of the music, so calculate the duration
        // of the trill extension line from the position of the last
        // note.
        hum::HumNum tstamp2 = getMeasureTstampPlusDur(lastnote, staffindex);
        int measures = getMeasureDifference(token, bartok);
        std::pair<int, double> ts2(measures, tstamp2.getFloat());
        trill->SetTstamp2(ts2);
        return;
    }

    bool isgracenote = endtok->find('q') == std::string::npos ? false : true;
    bool isrest = endtok->isRest();

    if (isgracenote) {
        trill->SetEndid("#" + getLocationId("note", endtok, -1));
    }
    else if (isrest && !foundbarline) {
        // use @tstamp2 because @endid does not work in verovio for rests.
        hum::HumNum tstamp2 = getMeasureTstampPlusDur(lasttok, staffindex);
        int measures = getMeasureDifference(token, lasttok);
        std::pair<int, double> ts2(measures, tstamp2.getFloat());
        trill->SetTstamp2(ts2);
    }
    else if (foundbarline && (barlinecount == 1) && nextnoteafterbarline) {
        // end trill extender before barline
        hum::HumNum tstamp2 = getMeasureTstampPlusDur(bartok->getPreviousToken(), staffindex);
        int measures = 0;
        std::pair<int, double> ts2(measures, tstamp2.getFloat());
        trill->SetTstamp2(ts2);
    }
    else {
        // The line extension crosses a barline, so need to use @tstamp2;
        // otherwise verovio will not display the line extension.
        hum::HumNum tstamp2 = getMeasureTstampPlusDur(lasttok, staffindex);
        int measures = getMeasureDifference(token, lasttok);
        std::pair<int, double> ts2(measures, tstamp2.getFloat());
        trill->SetTstamp2(ts2);
    }
    // Need to add another case when extender crosses a barline and ends before
    // a grace note.  This requires @endid since @tstamp2 will not work, as that
    // will extend to the note after the grace note(s) at the same tstamp.
}

//////////////////////////////
//
// HumdrumInput::processTieStart -- linked slurs not allowed in chords yet.
//

void HumdrumInput::processTieStart(Note *note, hum::HTp token, const std::string &tstring, int subindex)
{
    if (token->isMensLike()) {
        return;
    }
    std::string endtag = "tieEnd";
    if (subindex >= 0) {
        endtag += to_string(subindex + 1);
    }
    hum::HTp tieend = token->getValueHTp("auto", endtag);
    if (tieend) {
        // A linked tie which can be inserted immediately (and
        // not stored in the list of tie starts for later processing).

        std::string endnumtag = "tieEndSubtokenNumber";
        int endn = subindex + 1;
        if (token->isChord()) {
            if (endn > 0) {
                endnumtag += to_string(endn);
            }
        }
        int endnumber = token->getValueInt("auto", endnumtag);
        if (endnumber <= 0) {
            endnumber = 1;
        }

        vrv::Tie *tie = new Tie();

        addTieLineStyle(tie, token, subindex);

        addChildMeasureOrSection(tie);
        int endsubindex = endnumber - 1;
        if (endsubindex < 0) {
            endsubindex = 0;
        }
        setTieLocationId(tie, token, subindex, tieend, endsubindex);
        std::string startid = getLocationId("note", token);
        std::string endid = getLocationId("note", tieend);
        if (token->isChord()) {
            startid += "S" + to_string(subindex + 1);
        }
        if (tieend->isChord()) {
            if (endnumber > 0) {
                endid += "S" + to_string(endnumber);
            }
        }
        tie->SetStartid("#" + startid);
        tie->SetEndid("#" + endid);

        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum timestamp = token->getDurationFromStart();
    hum::HumNum endtime = timestamp + token->getDuration();
    int track = token->getTrack();
    int rtrack = m_rkern[track];
    std::string noteid = note->GetID();
    int cl = m_currentlayer;
    int pitch = hum::Convert::kernToMidiNoteNumber(tstring);

    int metertop = ss[rtrack].meter_top;
    hum::HumNum meterbot = ss[rtrack].meter_bottom;

    ss[rtrack].tiestarts.emplace_back();
    ss[rtrack].tiestarts.back().setStart(
        noteid, m_measure, cl, tstring, pitch, timestamp, endtime, subindex, token, metertop, meterbot);

    if (m_signifiers.above) {
        std::string marker = "[";
        if (tstring.find("_") != std::string::npos) {
            marker = "_";
        }
        marker.push_back(m_signifiers.above);
        if (tstring.find(marker) != std::string::npos) {
            ss[rtrack].tiestarts.back().setTieAbove();
        }
    }
    if (m_signifiers.below) {
        std::string marker = "[";
        if (tstring.find("_") != std::string::npos) {
            marker = "_";
        }
        marker.push_back(m_signifiers.below);
        if (tstring.find(marker) != std::string::npos) {
            ss[rtrack].tiestarts.back().setTieBelow();
        }
    }
}

//////////////////////////////
//
// processTieEnd --
//

void HumdrumInput::processTieEnd(Note *note, hum::HTp token, const std::string &tstring, int subindex)
{
    if (token->isMensLike()) {
        return;
    }
    std::string starttag = "tieStart";
    if (token->isChord()) {
        starttag += to_string(subindex + 1);
    }
    hum::HTp tiestart = token->getValueHTp("auto", starttag);
    if (tiestart) {
        // linked ties are handled in processTieStart().
        return;
    }

    std::vector<humaux::StaffStateVariables> &ss = m_staffstates;
    hum::HumNum timestamp = token->getDurationFromStart();
    int track = token->getTrack();
    int staffindex = m_rkern[track];
    std::string noteid = note->GetID();
    bool disjunct = token->find("]]") != std::string::npos;
    if (token->find("__") != std::string::npos) {
        disjunct = true;
    }

    int pitch = hum::Convert::kernToMidiNoteNumber(tstring);
    int layer = m_currentlayer;
    auto found = ss[staffindex].tiestarts.end();

    // search for open tie in current layer
    for (auto it = ss[staffindex].tiestarts.begin(); it != ss[staffindex].tiestarts.end(); ++it) {
        if (it->getLayer() != layer) {
            continue;
        }
        if (it->getPitch() != pitch) {
            continue;
        }
        if (disjunct && (it->getStartTokenPointer()->find("[[") != std::string::npos)) {
            found = it;
            break;
        }
        if (disjunct && (it->getStartTokenPointer()->find("__") != std::string::npos)) {
            found = it;
            break;
        }
        else if (it->getEndTime() == timestamp) {
            found = it;
            break;
        }
    }

    // search for open tie in current staff outside of current layer.
    if (found == ss[staffindex].tiestarts.end()) {
        for (auto it = ss[staffindex].tiestarts.begin(); it != ss[staffindex].tiestarts.end(); ++it) {
            if (it->getPitch() != pitch) {
                continue;
            }
            if (disjunct && (it->getStartTokenPointer()->find("[[") != std::string::npos)) {
                found = it;
                break;
            }
            else if (it->getEndTime() == timestamp) {
                found = it;
                break;
            }
        }
    }

    if (found == ss[staffindex].tiestarts.end()) {
        // can't find start of slur so give up.
        processHangingTieEnd(note, token, tstring, subindex, ss[staffindex].meter_bottom);
        return;
    }
    hum::HTp starttoken = found->getStartTokenPointer();

    bool needToBreak = inDifferentEndings(starttoken, token);
    if (needToBreak) {
        processHangingTieEnd(note, token, tstring, subindex, ss[staffindex].meter_bottom);
        return;
    }

    bool invisibleTieQ = false;
    if (starttoken && (subindex < 0) && !needToBreak) {
        // Only dealing with hiding invisible ties on single notes for now.
        if ((starttoken->find("[y") != std::string::npos) || (starttoken->find("_y") != std::string::npos)
            || (starttoken->find("_y") != std::string::npos)) {
            appendTypeTag(note, "no-attack");
            invisibleTieQ = true;
        }
    }

    if (!invisibleTieQ) {
        int metertop = ss[staffindex].meter_top;
        hum::HumNum meterbot = ss[staffindex].meter_bottom;
        hum::HumNum starttime = token->getDurationFromStart();
        hum::HumNum duration = hum::Convert::recipToDuration(token);
        hum::HumNum endtime = starttime + duration;

        Tie *tie = found->setEndAndInsert(
            noteid, m_measure, layer, tstring, pitch, starttime, endtime, subindex, token, metertop, meterbot);

        int startindex = found->getStartSubindex();
        if (starttoken) {
            addTieLineStyle(tie, starttoken, startindex);
        }
        setTieLocationId(tie, starttoken, startindex, token, subindex);
        if (found->isInserted()) {
            // Only deleting the finished tie if it was successful.  Undeleted
            // ones can be checked later.  They are either encoding errors, or
            // hanging ties, or arpeggiation ties (the latter should be encoded
            // with [[, ]] rather than [, ]).
            ss[staffindex].tiestarts.erase(found);
        }
    }
    else {
        ss[staffindex].tiestarts.erase(found);
    }
}

/////////////////////////////
//
// HumdrumInput::characterCount --
//

int HumdrumInput::characterCount(const std::string &text, char symbol)
{
    return (int)std::count(text.begin(), text.end(), symbol);
}

int HumdrumInput::characterCount(hum::HTp token, char symbol)
{
    return (int)std::count(token->begin(), token->end(), symbol);
}

/////////////////////////////
//
// HumdrumInput::characterCountInSubtoken --
//

int HumdrumInput::characterCountInSubtoken(const std::string &text, char symbol)
{
    int sum = 0;
    for (int i = 0; i < (int)text.size(); ++i) {
        if (text[i] == symbol) {
            sum++;
        }
        if (text[i] == ' ') {
            break;
        }
    }
    return sum;
}

int HumdrumInput::characterCountInSubtoken(hum::HTp token, char symbol)
{
    return characterCountInSubtoken(*token, symbol);
}

/////////////////////////////
//
// HumdrumInput::printMeasureTokens -- For debugging.
//

void HumdrumInput::printMeasureTokens()
{
    std::vector<std::vector<std::vector<hum::HTp>>> &lt = m_layertokens;
    int i, j, k;
    cerr << endl;
    for (i = 0; i < (int)lt.size(); ++i) {
        cerr << "STAFF " << i + 1 << "\t";
        for (j = 0; j < (int)lt[i].size(); ++j) {
            cerr << "LAYER " << j + 1 << ":\t";
            for (k = 0; k < (int)lt[i][j].size(); ++k) {
                cout << " " << *lt[i][j][k];
            }
            cerr << endl;
        }
    }
}

/////////////////////////////
//
// HumdrumInput::setDuration --  Incoming duration is in quarter notes units.
//    This function needs to be generalized (and how to deal with tuplets)?
//    Mostly for <space> elements.
//

template <class ELEMENT> hum::HumNum HumdrumInput::setDuration(ELEMENT element, hum::HumNum duration)
{

    if (duration == 3) {
        element->SetDur(DURATION_2);
        element->SetDots(1);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 2)) {
        element->SetDur(DURATION_8);
        return duration;
    }
    else if ((duration.getNumerator() == 3) && (duration.getDenominator() == 2)) {
        element->SetDur(DURATION_4);
        element->SetDots(1);
        return duration;
    }
    else if ((duration.getNumerator() == 3) && (duration.getDenominator() == 4)) {
        element->SetDur(DURATION_8);
        element->SetDots(1);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 4)) {
        element->SetDur(DURATION_16);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 8)) {
        element->SetDur(DURATION_32);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 16)) {
        element->SetDur(DURATION_64);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 32)) {
        element->SetDur(DURATION_128);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 64)) {
        element->SetDur(DURATION_256);
        return duration;
    }
    else if ((duration.getNumerator() == 1) && (duration.getDenominator() == 128)) {
        element->SetDur(DURATION_512);
        return duration;
    }
    if (duration >= 16) {
        element->SetDur(DURATION_long);
        return 16;
    }
    if (duration >= 8) {
        element->SetDur(DURATION_breve);
        return 8;
    }
    if (duration >= 4) {
        element->SetDur(DURATION_1);
        return 4;
    }
    if (duration >= 2) {
        element->SetDur(DURATION_2);
        return 2;
    }
    if (duration >= 1) {
        element->SetDur(DURATION_4);
        return 1;
    }
    // Don't know what to do, so return duration
    // There will be an error in the data.
    cerr << "Unprintable rhythm: " << duration << endl;
    return duration;
}

//////////////////////////////
//
// HumdrumInput::tieToPreviousItem -- Tie a note to the start of
//    the measure (presumably the note is at the start of the measure
//    as well).
//

Tie *HumdrumInput::tieToPreviousItem(hum::HTp token, int subindex, hum::HumNum meterunit, Measure *measure)
{
    Tie *tie = new Tie();
    addTieLineStyle(tie, token, subindex);
    addChildMeasureOrSection(tie, measure);
    hum::HTp starttoken = token->getOwner()->getTrackStart(token->getTrack());
    hum::HTp current = token->getPreviousToken();
    while (current) {
        // match to previous barline if possible
        if (current->isBarline()) {
            break;
        }
        if (current->isInterpretation()) {
            if (current->compare(0, 2, "**") == 0) {
                break; // exclusive interpretation (start of spine)
            }
        }
        if (current->isData() && !current->isNull()) {
            // What about grace notes preceeding a tie end?
            break;
        }
        current = current->getPreviousToken();
    }
    if (current) {
        starttoken = current;
    }

    setTieLocationId(tie, starttoken, -1, token, subindex);

    std::string endid = getLocationId("note", token);
    if (token->isChord()) {
        int endnumber = subindex + 1;
        if (endnumber > 0) {
            endid += "S" + to_string(endnumber);
        }
    }

    // Currently a bug in verovio for @tstamp=0, so
    // make an adjustment to compensate relative to meter.unit:
    // int tstamp = 0;
    hum::HumNum tstamp;
    if (current->isBarline() || current->isInterpretation()) {
        hum::HumNum tstamp = meterunit;
        tstamp /= 4;
        tstamp = -tstamp + 1;
        if (tstamp < 0) {
            tstamp = 0;
        }
    }
    else if (current->isData()) {
        hum::HumNum frombar = starttoken->getDurationFromBarline();
        tstamp = frombar;
        tstamp *= meterunit;
        tstamp /= 4;
        tstamp += 1;
    }
    else {
        cerr << "STRANGE CASE IN TIE INSERTION" << endl;
    }

    tie->SetTstamp(tstamp.getFloat()); // attach start to beginning of measure
    tie->SetEndid("#" + endid);

    return tie;
}

//////////////////////////////
//
// HumdrumInput::getStaffLayerCounts -- Return the maximum layer count for each
//    part within the measure.
//

std::vector<int> HumdrumInput::getStaffLayerCounts()
{
    std::vector<std::vector<std::vector<hum::HTp>>> &lt = m_layertokens;
    std::vector<int> output(lt.size(), 0);

    int i;
    for (i = 0; i < (int)lt.size(); ++i) {
        output[i] = (int)lt[i].size();
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::setupSystemMeasure -- prepare a new system measure.
//   Also checks if the key or time signatures change at the start
//   of the measures (other than the first measure of a score).
//

void HumdrumInput::setupSystemMeasure(int startline, int endline)
{
    hum::HumdrumFile &infile = m_infiles[0];

    if (m_oclef.size() || m_omet.size() || m_okey.size()) {
        storeOriginalClefMensurationKeyApp();
    }

    // fix for **mens ggg

    if (!m_mens && (infile[startline].getDurationFromStart() > 0)) {
        addSystemClefKeyTimeChange(startline, endline);
    }

    std::string previoussection = m_lastsection;
    std::string currentsection;
    if (m_sectionlabels[startline]) {
        currentsection = *m_sectionlabels[startline];
        if (currentsection.compare(0, 2, "*>") == 0) {
            currentsection = currentsection.substr(2);
        }
        currentsection = "label-" + currentsection;
    }
    else {
        currentsection = "";
    }

    if (hasMensuralStaff(&infile[startline])) {
        m_measure = new Measure(false);
    }
    else {
        m_measure = new Measure();
    }

    int endnum = 0;
    bool ending = false;
    bool fakenum = false;
    bool newsection = false;
    if ((currentsection.size() > 0) && isdigit(currentsection.back())) {
        hum::HTp withnum = m_sectionlabels[startline];
        hum::HTp nonum = m_numberlesslabels[startline];
        std::string withnumstr;
        std::string nonumstr;
        if (withnum) {
            withnumstr = *withnum;
        }
        if (nonum) {
            nonumstr = *nonum;
        }
        if ((!nonumstr.empty()) && (withnumstr.compare(0, nonumstr.size(), nonumstr) == 0)) {
            ending = true;
        }
        else {
            fakenum = true;
            ending = false;
        }
        if (ending) {
            std::smatch matches;
            if (regex_search(currentsection, matches, regex("(\\d+)$"))) {
                endnum = stoi(matches[1]);
            }
            else {
                endnum = 0;
            }
        }
        else {
            endnum = 0;
        }
    }

    else if (currentsection != m_lastsection) {
        newsection = true;
        if (m_lastsection != currentsection) {
            if (m_sections.size() > 1) {
                // keep movement-level section in stack.
                m_sections.pop_back();
            }
        }
        if (m_sectionlabels[startline]) {
            m_lastsection = currentsection;
        }
        else {
            m_lastsection = "";
        }
    }

    if (ending && (m_endingnum != endnum)) {
        // create a new ending
        m_currentending = new Ending();
        std::string endingid = *m_sectionlabels[startline];
        if (endingid.compare(0, 2, "*>") == 0) {
            endingid = endingid.substr(2);
        }
        endingid = "label-" + endingid;
        setN(m_currentending, endnum, m_sectionlabels[startline]);
        // sanitize id if not valid:
        m_currentending->SetID(endingid);
        if (m_sections.size() > 1) {
            // assuming the ending does not start at beginning
            // of music.
            m_sections.pop_back();
        }
        m_sections.back()->AddChild(m_currentending);
        m_currentending->AddChild(m_measure);
    }
    else if ((!fakenum) && (currentsection.size() > 0) && isdigit(currentsection.back())) {
        // inside a current ending
        m_currentending->AddChild(m_measure);
    }
    else if (newsection) {
        // start a new section
        m_currentending = NULL;
        m_currentsection = new Section();
        if (m_measure) {
            m_currentsection->AddChild(m_measure);
        }
        m_currentsection->SetID(m_lastsection);
        m_sections.back()->AddChild(m_currentsection);
        m_sections.push_back(m_currentsection);
    }
    else {
        // outside of an ending
        m_currentending = NULL;
        if (m_measure) {
            m_sections.back()->AddChild(m_measure);
        }
    }

    m_endingnum = endnum;
    m_measures.push_back(m_measure);

    if (m_leftbarstyle != BARRENDITION_NONE) {
        if (m_measure) {
            m_measure->SetLeft(m_leftbarstyle);
        }
        m_leftbarstyle = BARRENDITION_NONE;
    }

    if (m_measure) {
        setLocationId(m_measure, startline, -1, -1);
    }

    int measurenumber = getMeasureNumber(startline, endline);
    if (measurenumber >= 0) {
        if (m_measure) {
            setN(m_measure, measurenumber);
        }
    }

    if (m_doc->GetOptions()->m_humType.GetValue()) {
        if (m_measure) {
            std::stringstream measuretag;
            measuretag << "m-" << measurenumber;
            appendTypeTag(m_measure, measuretag.str());
        }
    }

    if (m_measure) {
        setSystemMeasureStyle(startline, endline);
    }
}

//////////////////////////////
//
// HumdrumInput::hasMensuralStaff --  Return true if any **mens spines.
//

bool HumdrumInput::hasMensuralStaff(hum::HLp line)
{
    for (int i = 0; i < line->getFieldCount(); ++i) {
        if (line->token(i)->isMensLike()) {
            return true;
        }
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::checkForRehearsal -- Only attached to barlines for now.
//     Also required to be global layout for now, add note attachment
//     later.
//

void HumdrumInput::checkForRehearsal(int line)
{
    hum::HumdrumFile &infile = m_infiles[0];
    if (!infile[line].isBarline()) {
        return;
    }

    hum::HTp token = infile.token(line, 0);
    int lcount = token->getLinkedParameterSetCount();
    for (int i = 0; i < lcount; ++i) {
        bool globalQ = token->linkedParameterIsGlobal(i);
        if (!globalQ) {
            continue;
        }
        hum::HumParamSet *hps = token->getLinkedParameterSet(i);
        if (hps == NULL) {
            return;
        }
        if (hps->getNamespace1() != "LO") {
            return;
        }
        std::string namespace2 = hps->getNamespace2();
        bool rehQ = namespace2 == "REH";
        if (!rehQ) {
            continue;
        }
        std::string tvalue;
        std::string key;
        std::string value;
        for (int j = 0; j < hps->getCount(); ++j) {
            key = hps->getParameterName(j);
            value = hps->getParameterValue(j);
            if (key == "t") {
                tvalue = value;
                break;
            }
        }
        if (tvalue.empty()) {
            continue;
        }
        Reh *reh = new Reh();
        Rend *rend = new Rend();
        Text *text = new Text();
        std::u32string wtext = UTF8to32(tvalue);
        text->SetText(wtext);
        reh->AddChild(rend);
        rend->AddChild(text);
        rend->SetRend(TEXTRENDITION_box);
        addChildMeasureOrSection(reh);
        // Add to top staff for now, but add to top of
        // each instrumentalgoup probably in the future.
        setStaff(reh, 1);
        // Only attached to barline for now:
        reh->SetTstamp(0.0);
    }
}

//////////////////////////////
//
// HumdrumInput::addFTremSlurs --
//

void HumdrumInput::addFTremSlurs()
{
    if (m_ftrem_slurs.empty()) {
        return;
    }
    if (!m_measure) {
        return;
    }
    for (int i = 0; i < (int)m_ftrem_slurs.size(); ++i) {
        addChildBackMeasureOrSection(m_ftrem_slurs.at(i));
    }
    m_ftrem_slurs.clear();
}

//////////////////////////////
//
// HumdrumInput::storeOriginalClefMensurationKeyApp -- If there are any original
// clefs or mensuration signs, create an app for them.
//
// <app>
//    <lem></lem>
//    <rdg label="original">
//       <scoreDef>
//          <staffGrp>
//             <staffDef clef.shape="C" clef.line="4" n="1"/>
//          </staffGrp>
//       </scoreDef>
//    </rdg>
// </app>
//

void HumdrumInput::storeOriginalClefMensurationKeyApp()
{
    if ((1)) {
        // currently deactivated (use modori filter to switch between
        // modern and original clef/key/mensuration).
        return;
    }
    int staffindex = m_currentstaff - 1;
    if (m_oclef.empty() && m_omet.empty() && m_okey.empty()) {
        return;
    }

    int kerncount = 0;
    // int menscount = 0;
    for (int i = 0; i < (int)m_oclef.size(); ++i) {
        if (m_oclef[i].second->isKernLike()) {
            kerncount++;
        }
        else if (m_oclef[i].second->isMensLike()) {
            // menscount++;
        }
    }

    if (kerncount == 0) {
        // Don't produce <app> for **mens data for now.
    }

    App *app = new App();
    m_sections.back()->AddChild(app);

    Lem *lem = new Lem();
    app->AddChild(lem);

    Rdg *rdg = new Rdg();
    app->AddChild(rdg);
    rdg->SetLabel("original-clef");

    ScoreDef *scoredef = new ScoreDef();
    rdg->AddChild(scoredef);

    StaffGrp *staffgrp = new StaffGrp();
    scoredef->AddChild(staffgrp);

    if (m_oclef.size() > 0) {
        for (int i = 0; i < (int)m_oclef.size(); ++i) {
            StaffDef *staffdef = new StaffDef();
            staffgrp->AddChild(staffdef);
            setClef(staffdef, *m_oclef[i].second);
            staffdef->SetN(m_oclef[i].first);
            for (int j = 0; j < (int)m_omet.size(); ++j) {
                if (m_omet[j].first != m_oclef[i].first) {
                    continue;
                }
                setMeterSymbol(staffdef, *m_omet[j].second, staffindex);
            }
            for (int j = 0; j < (int)m_okey.size(); ++j) {
                if (m_okey[j].first != m_oclef[i].first) {
                    continue;
                }
                setKeySig(staffdef, m_okey[j].second);
            }
        }
    }

    else if (m_omet.size() > 0) {
        // No oclefs, just omets and maybe okeys.
        for (int i = 0; i < (int)m_oclef.size(); ++i) {
            StaffDef *staffdef = new StaffDef();
            staffgrp->AddChild(staffdef);
            setMeterSymbol(staffdef, *m_omet[i].second, staffindex);
            staffdef->SetN(m_omet[i].first);
            for (int j = 0; j < (int)m_okey.size(); ++j) {
                if (m_okey[j].first != m_omet[i].first) {
                    continue;
                }
                setKeySig(staffdef, m_okey[j].second);
            }
        }
    }

    else if (m_okey.size() > 0) {
        // No oclefs or omets, just okeys.
        for (int i = 0; i < (int)m_okey.size(); ++i) {
            StaffDef *staffdef = new StaffDef();
            staffgrp->AddChild(staffdef);
            setKeySig(staffdef, m_okey[i].second);
            staffdef->SetN(m_okey[i].first);
        }
    }

    m_oclef.clear();
    m_omet.clear();
}

//////////////////////////////
//
// HumdrumInput::setSystemMeasureStyle -- Set the style of the left and/or
//    right barline for the measure.
//
// Bar types listed in ../libmei/atttypes.h
//     BARRENDITION_NONE = 0,
//     BARRENDITION_dashed,
//     BARRENDITION_dotted,
//     BARRENDITION_dbl,
//     BARRENDITION_dbldashed,
//     BARRENDITION_dbldotted,
//     BARRENDITION_end,
//     BARRENDITION_invis,
//     BARRENDITION_rptstart,
//     BARRENDITION_rptboth,
//     BARRENDITION_rptend,
//     BARRENDITION_single,
// See also:
//      http://music-encoding.org/documentation/2.1.1/data.BARRENDITION
//      http://music-encoding.org/guidelines/3.0.0/Images/ExampleImages/barline-20100510.png
//

void HumdrumInput::setSystemMeasureStyle(int startline, int endline)
{
    hum::HumdrumFile &infile = m_infiles[0];

    hum::HTp token = infile.token(endline, 0);
    if (!token->isBarline()) {
        m_measure->SetRight(BARRENDITION_invis);
        return;
    }

    if (!infile[endline].allSameBarlineStyle()) {
        // non-invisible staff barlines will be added later.
        m_measure->SetRight(BARRENDITION_invis);
        return;
    }

    std::string endbar = infile[endline].getTokenString(0);
    std::string startbar = infile[startline].getTokenString(0);

    if (endbar.compare(0, 2, "==") == 0) {
        m_measure->SetRight(BARRENDITION_end);
    }
    else if (endbar.find(":|!|:") != std::string::npos) {
        // m_measure->SetRight(BARRENDITION_rptboth);
        m_measure->SetRight(BARRENDITION_rptend);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find(":!!:") != std::string::npos) {
        // m_measure->SetRight(BARRENDITION_rptboth);
        m_measure->SetRight(BARRENDITION_rptend);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find(":||:") != std::string::npos) {
        // m_measure->SetRight(BARRENDITION_rptboth);
        m_measure->SetRight(BARRENDITION_rptend);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find(":!:") != std::string::npos) {
        // m_measure->SetRight(BARRENDITION_rptboth);
        m_measure->SetRight(BARRENDITION_rptend);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find(":|:") != std::string::npos) {
        // m_measure->SetRight(BARRENDITION_rptboth);
        m_measure->SetRight(BARRENDITION_rptend);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find(":|") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_rptend);
    }
    else if (endbar.find(":!") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_rptend);
    }
    else if (startbar.find("!:") != std::string::npos) {
        // m_measure->SetLeft(BARRENDITION_rptstart);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find("|:") != std::string::npos) {
        // m_measure->SetLeft(BARRENDITION_rptstart);
        setNextLeftBarStyle(BARRENDITION_rptstart);
    }
    else if (endbar.find("||") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_dbl);
        // setNextLeftBarStyle(BARRENDITION_dbl);
    }
    else if (endbar.find("-") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_invis);
    }
    else if (endbar.find("::") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_dbldashed);
    }
    else if (endbar.find(":") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_dashed);
    }
    else if (endbar.find("..") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_dbldotted);
    }
    else if (endbar.find(".") != std::string::npos) {
        m_measure->SetRight(BARRENDITION_dotted);
    }
}

//////////////////////////////
//
// HumdrumInput::setNextLeftBarStyle -- Store a bar style to apply
//      on the left side of the next measure.
//

void HumdrumInput::setNextLeftBarStyle(data_BARRENDITION style)
{
    m_leftbarstyle = style;
}

//////////////////////////////
//
// HumdrumInput::getMeasureEndLine -- Return the line index of the
//   ending of a given measure.  This is usually a barline, but can be
//   the end of a file if there is no terminal barline in the **kern
//   data.  Returns a negative line number if there is no data in the
//   measure.
//

int HumdrumInput::getMeasureEndLine(int startline)
{
    hum::HumdrumFile &infile = m_infiles[0];
    int endline = infile.getLineCount() - 1;
    bool foundDataQ = false;
    int i = startline + 1;
    while (i < infile.getLineCount()) {
        if (infile[i].isData()) {
            foundDataQ = true;
        }
        else if (infile[i].isBarline()) {
            // If the barlines are not all of the same style, then
            // treat the barline as <barLine> rather than create new <measure>:
            if (infile[i].allSameBarlineStyle()) {
                endline = i;
                break;
            }
            else if (!infile[i].hasDataStraddle()) {
                endline = i;
                break;
            }
        }
        endline = i;
        i++;
    }

    if (foundDataQ) {
        return endline;
    }
    else {
        return -endline;
    }
}

//////////////////////////////
//
// HumdrumInput::setupMeiDocument -- Add a page and a system on the page to
//     get things started.
//

void HumdrumInput::setupMeiDocument()
{
    hum::HumdrumFile &infile = m_infiles[0];
    m_doc->Reset();
    m_doc->SetType(Raw);
    // The mdiv
    Mdiv *mdiv = new Mdiv();
    mdiv->m_visibility = Visible;
    m_doc->AddChild(mdiv);
    // The score
    m_score = new Score();
    mdiv->AddChild(m_score);
    // the section
    Section *section = new Section();
    hum::HTp starting = infile.getTrackStart(1);
    if (starting) {
        section->SetID(getLocationId(section, starting, -1));
        storeExpansionLists(section, starting);
    }
    m_sections.push_back(section);
    m_score->AddChild(m_sections.back());
    m_leftbarstyle = BARRENDITION_NONE;
    if (m_breaks) {
        // An initial page break is required in order for the system
        // breaks encoded in the file to be activated, so adding a
        // dummy page break here:
        Pb *pb = new Pb;
        m_layoutInformation = LAYOUT_ENCODED;
        section->AddChild(pb);
    }
}

//////////////////////////////
//
// HumdrumInput::clear -- clear contents of object (m_doc handled by
//    parent class).
//

void HumdrumInput::clear()
{
    m_filename = "";
    m_tupletscaling = 1;
    m_breaks = false;
    m_duradj.clear();
    m_nulls.clear();
    m_fbstates.clear();
    for (int i = 0; i < (int)m_scordatura_transposition.size(); ++i) {
        delete m_scordatura_transposition[i];
        m_scordatura_transposition[i] = NULL;
    }
    m_scordatura_transposition.clear();
}

//////////////////////////////
//
// HumdrumInput::getMeasureNumber -- Return the current barline's measure
//     number, or return -1 if no measure number.  Returns 0 if a
//     pickup measure.
//

int HumdrumInput::getMeasureNumber(int startline, int endline)
{
    hum::HumdrumFile &infile = m_infiles[0];

    int number;
    if (infile[startline].isBarline()) {
        if (sscanf(infile[startline].getTokenString(0).c_str(), "=%d", &number) == 1) {
            return number;
        }
        else {
            return -1;
        }
    }
    else {
        // at the start of the score (most likely).  Search through the first
        // spine for the first barline, provided that it starts before the first
        // data line.
        bool found = false;
        int linenum = -1;
        for (int i = 0; i < infile.getLineCount(); ++i) {
            if (infile[i].isBarline()) {
                found = true;
                linenum = i;
                break;
            }
            else if (infile[i].isData()) {
                found = false;
                linenum = -1;
                // if (infile.hasPickup()) {
                //         // set the first implicit measure to 0
                //         return 0;
                // }
                break;
            }
        }
        if (found) {
            if (sscanf(infile[linenum].getTokenString(0).c_str(), "=%d", &number) == 1) {
                return number;
            }
            else {
                return -1;
            }
        }
        else {
            return -1;
        }
    }
}

//////////////////////////////
//
// HumdrumInput::calculateLayout -- Have verovio figure out the
//     line break (and page break) locations in the data.  Do this
//     after the Humdrum data has finished loading.
//

void HumdrumInput::calculateLayout()
{
    // m_doc->CastOff();
}

//////////////////////////////
//
// HumdrumInput::emptyMeasures -- For initial development, maybe convert to
//     an option.
//

bool HumdrumInput::emptyMeasures()
{
    return false;
}

//////////////////////////////
//
// HumdrumInput::unescapeHtmlEntities -- from
//    http://www.codecodex.com/wiki/Unescape_HTML_special_characters_from_a_String
//

typedef struct {
    const char *Name;
    unsigned int Value;
} EntityNameEntry;

// list of entity names defined in HTML 4.0 spec
static const EntityNameEntry StaticEntityNames[] = {
    { "nbsp", 160 }, { "iexcl", 161 }, { "cent", 162 }, { "pound", 163 }, { "curren", 164 }, { "yen", 165 },
    { "brvbar", 166 }, { "sect", 167 }, { "uml", 168 }, { "copy", 169 }, { "ordf", 170 }, { "laquo", 171 },
    { "not", 172 }, { "shy", 173 }, { "reg", 174 }, { "macr", 175 }, { "deg", 176 }, { "plusmn", 177 }, { "sup2", 178 },
    { "sup3", 179 }, { "acute", 180 }, { "micro", 181 }, { "para", 182 }, { "middot", 183 }, { "cedil", 184 },
    { "sup1", 185 }, { "ordm", 186 }, { "raquo", 187 }, { "frac14", 188 }, { "frac12", 189 }, { "frac34", 190 },
    { "iquest", 191 }, { "Agrave", 192 }, { "Aacute", 193 }, { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 },
    { "Aring", 197 }, { "AElig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 }, { "Eacute", 201 }, { "Ecirc", 202 },
    { "Euml", 203 }, { "Igrave", 204 }, { "Iacute", 205 }, { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 }, { "Otilde", 213 }, { "Ouml", 214 },
    { "times", 215 }, { "Oslash", 216 }, { "Ugrave", 217 }, { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 },
    { "Yacute", 221 }, { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 }, { "aacute", 225 }, { "acirc", 226 },
    { "atilde", 227 }, { "auml", 228 }, { "aring", 229 }, { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 },
    { "eacute", 233 }, { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 }, { "iacute", 237 }, { "icirc", 238 },
    { "iuml", 239 }, { "eth", 240 }, { "ntilde", 241 }, { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 },
    { "otilde", 245 }, { "ouml", 246 }, { "divide", 247 }, { "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 },
    { "ucirc", 251 }, { "uuml", 252 }, { "yacute", 253 }, { "thorn", 254 }, { "yuml", 255 }, { "fnof", 402 },
    /* Greek */ { "Alpha", 913 }, { "Beta", 914 }, { "Gamma", 915 }, { "Delta", 916 }, { "Epsilon", 917 },
    { "Zeta", 918 }, { "Eta", 919 }, { "Theta", 920 }, { "Iota", 921 }, { "Kappa", 922 }, { "Lambda", 923 },
    { "Mu", 924 }, { "Nu", 925 }, { "Xi", 926 }, { "Omicron", 927 }, { "Pi", 928 }, { "Rho", 929 }, { "Sigma", 931 },
    { "Tau", 932 }, { "Upsilon", 933 }, { "Phi", 934 }, { "Chi", 935 }, { "Psi", 936 }, { "Omega", 937 },
    { "alpha", 945 }, { "beta", 946 }, { "gamma", 947 }, { "delta", 948 }, { "epsilon", 949 }, { "zeta", 950 },
    { "eta", 951 }, { "theta", 952 }, { "iota", 953 }, { "kappa", 954 }, { "lambda", 955 }, { "mu", 956 },
    { "nu", 957 }, { "xi", 958 }, { "omicron", 959 }, { "pi", 960 }, { "rho", 961 }, { "sigmaf", 962 },
    { "sigma", 963 }, { "tau", 964 }, { "upsilon", 965 }, { "phi", 966 }, { "chi", 967 }, { "psi", 968 },
    { "omega", 969 }, { "thetasym", 977 }, { "upsih", 978 }, { "piv", 982 },
    /* General Punctuation */
    { "bull", 8226 }, { "hellip", 8230 }, { "prime", 8242 }, { "Prime", 8243 }, { "oline", 8254 }, { "frasl", 8260 },
    /* Letterlike Symbols */
    { "weierp", 8472 }, { "image", 8465 }, { "real", 8476 }, { "trade", 8482 }, { "alefsym", 8501 },
    /* Arrows */
    { "larr", 8592 }, { "uarr", 8593 }, { "rarr", 8594 }, { "darr", 8595 }, { "harr", 8596 }, { "crarr", 8629 },
    { "lArr", 8656 }, { "uArr", 8657 }, { "rArr", 8658 }, { "dArr", 8659 }, { "hArr", 8660 },
    /* Mathematical Operators */
    { "forall", 8704 }, { "part", 8706 }, { "exist", 8707 }, { "empty", 8709 }, { "nabla", 8711 }, { "isin", 8712 },
    { "notin", 8713 }, { "ni", 8715 }, { "prod", 8719 }, { "sum", 8721 }, { "minus", 8722 }, { "lowast", 8727 },
    { "radic", 8730 }, { "prop", 8733 }, { "infin", 8734 }, { "and", 8743 }, { "or", 8744 }, { "cap", 8745 },
    { "cup", 8746 }, { "int", 8747 }, { "there4", 8756 }, { "sim", 8764 }, { "cong", 8773 }, { "asymp", 8776 },
    { "ne", 8800 }, { "equiv", 8801 }, { "le", 8804 }, { "ge", 8805 }, { "sub", 8834 }, { "sup", 8835 },
    { "nsub", 8836 }, { "sube", 8838 }, { "supe", 8839 }, { "oplus", 8853 }, { "otimes", 8855 }, { "perp", 8869 },
    { "sdot", 8901 },
    /* Miscellaneous Technical */
    { "lceil", 8968 }, { "rceil", 8969 }, { "lfloor", 8970 }, { "rfloor", 8971 }, { "lang", 9001 }, { "rang", 9002 },
    /* Geometric Shapes */
    { "loz", 9674 },
    /* Miscellaneous Symbols */
    { "spades", 9824 }, { "clubs", 9827 }, { "hearts", 9829 }, { "diams", 9830 }, { "quot", 34 }, { "amp", 38 },
    { "lt", 60 }, { "gt", 62 },
    /* Latin Extended-A */
    { "OElig", 338 }, { "oelig", 339 }, { "Scaron", 352 }, { "scaron", 353 }, { "Yuml", 376 },
    /* Spacing Modifier Letters */
    { "circ", 710 }, { "tilde", 732 },
    /* General Punctuation */
    { "ensp", 8194 }, { "emsp", 8195 }, { "thinsp", 8201 }, { "zwnj", 8204 }, { "zwj", 8205 }, { "lrm", 8206 },
    { "rlm", 8207 }, { "ndash", 8211 }, { "mdash", 8212 }, { "lsquo", 8216 }, { "rsquo", 8217 }, { "sbquo", 8218 },
    { "ldquo", 8220 }, { "rdquo", 8221 }, { "bdquo", 8222 }, { "dagger", 8224 }, { "Dagger", 8225 }, { "permil", 8240 },
    { "lsaquo", 8249 }, { "rsaquo", 8250 }, { "euro", 8364 }, { NULL, 0 } /* marks end of list */
} /*StaticEntityNames*/;

typedef std::map<std::string, unsigned int> EntityNameMap;
typedef std::pair<std::string, unsigned int> EntityNamePair;
static EntityNameMap EntityNames;

//////////////////////////////
//
// HumdrumInput::unescapeHtmlEntities --
//

string HumdrumInput::unescapeHtmlEntities(const std::string &input)
{
    std::stringstream indata;
    indata << input;
    std::stringstream outdata;
    UnquoteHTML(indata, outdata);
    return outdata.str();
}

//////////////////////////////
//
// HumdrumInput::WriteUTF8 -- writes Ch in UTF-8 encoding to Out.
//     Note this version only deals with characters up to 16 bits.
//

void HumdrumInput::WriteUTF8(std::ostream &Out, unsigned int Ch)
{
    if (Ch >= 0x800) {
        Out.put(0xE0 | ((Ch >> 12) & 0x0F));
        Out.put(0x80 | ((Ch >> 6) & 0x3F));
        Out.put(0x80 | (Ch & 0x3F));
    }
    else if (Ch >= 0x80) {
        Out.put(0xC0 | ((Ch >> 6) & 0x1F));
        Out.put(0x80 | (Ch & 0x3F));
    }
    else {
        Out.put(Ch);
    } /*if*/
} /*WriteUTF8*/

//////////////////////////////
//
// HumdrumInput::UnquoteHTML -- Copies In to Out, expanding any HTML entity
//     references into literal UTF-8 characters.
//

void HumdrumInput::UnquoteHTML(std::istream &In, std::ostream &Out)
{
    enum {
        NoMatch,
        MatchBegin,
        MatchName,
        MatchNumber,
        MatchDecimalNumber,
        MatchHexNumber,
    } MatchState;
    std::string MatchingName;
    unsigned int CharCode = 0;
    bool ProcessedChar, GotCharCode;
    MatchState = NoMatch;
    for (;;) {
        const unsigned char ThisCh = In.get();
        if (In.eof()) break;
        ProcessedChar = false; /* to begin with */
        GotCharCode = false; /* to begin with */
        switch ((int)MatchState) {
            case (int)MatchBegin:
                if (ThisCh == '#') {
                    MatchState = MatchNumber;
                    ProcessedChar = true;
                }
                else if ((ThisCh >= 'a' && ThisCh <= 'z') || (ThisCh >= 'A' && ThisCh <= 'Z')) {
                    MatchingName.append(1, ThisCh);
                    MatchState = MatchName;
                    ProcessedChar = true;
                }
                else {
                    Out.put('&');
                    MatchState = NoMatch;
                }
                break;
            case (int)MatchName:
                if ((ThisCh >= 'a' && ThisCh <= 'z') || (ThisCh >= 'A' && ThisCh <= 'Z')
                    || (ThisCh >= '0' && ThisCh <= '9')) {
                    MatchingName.append(1, ThisCh);
                    ProcessedChar = true;
                }
                else if (ThisCh == ';') {
                    if (EntityNames.empty()) {
                        /* first use, load EntityNames from StaticEntityNames */
                        const EntityNameEntry *ThisEntry;
                        ThisEntry = StaticEntityNames;
                        for (;;) {
                            if (ThisEntry->Name == NULL) break;
                            EntityNames.insert(EntityNamePair(std::string(ThisEntry->Name), ThisEntry->Value));
                            ThisEntry++;
                        }
                    }
                    const EntityNameMap::const_iterator NameEntry = EntityNames.find(MatchingName);
                    if (NameEntry != EntityNames.end()) {
                        CharCode = NameEntry->second;
                        ProcessedChar = true;
                        GotCharCode = true;
                    }
                }
                if (!ProcessedChar) {
                    Out.put('&');
                    for (unsigned int i = 0; i < MatchingName.size(); ++i) {
                        Out.put(MatchingName[i]);
                    } /*for*/
                    MatchState = NoMatch;
                }
                break;
            case (int)MatchNumber:
                if (ThisCh == 'x' || ThisCh == 'X') {
                    ProcessedChar = true;
                    MatchState = MatchHexNumber;
                    CharCode = 0;
                }
                else if (ThisCh >= '0' && ThisCh <= '9') {
                    CharCode = ThisCh - '0';
                    MatchState = MatchDecimalNumber;
                    ProcessedChar = true;
                }
                else {
                    MatchState = NoMatch;
                }
                break;
            case (int)MatchDecimalNumber:
                if (ThisCh >= '0' && ThisCh <= '9') {
                    CharCode = CharCode * 10 + ThisCh - '0';
                    ProcessedChar = true;
                }
                else if (ThisCh == ';') {
                    ProcessedChar = true;
                    GotCharCode = true;
                }
                else {
                    MatchState = NoMatch;
                }
                break;
            case (int)MatchHexNumber:
                if (ThisCh >= '0' && ThisCh <= '9') {
                    CharCode = CharCode * 16 + ThisCh - '0';
                    ProcessedChar = true;
                }
                else if (ThisCh >= 'a' && ThisCh <= 'f') {
                    CharCode = CharCode * 16 + ThisCh - 'a' + 10;
                    ProcessedChar = true;
                }
                else if (ThisCh >= 'A' && ThisCh <= 'F') {
                    CharCode = CharCode * 16 + ThisCh - 'A' + 10;
                    ProcessedChar = true;
                }
                else if (ThisCh == ';') {
                    ProcessedChar = true;
                    GotCharCode = true;
                }
                else {
                    MatchState = NoMatch;
                }
                break;
        }
        if (GotCharCode) {
            WriteUTF8(Out, CharCode);
            MatchState = NoMatch;
        }
        else if (!ProcessedChar && MatchState == NoMatch) {
            if (ThisCh == '&') {
                MatchState = MatchBegin;
                MatchingName.erase();
            }
            else {
                Out.put(ThisCh);
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::GetMeiString -- Return the converted MEI content.
//

std::string HumdrumInput::GetMeiString()
{
    MEIOutput meioutput(m_doc);
    meioutput.SetScoreBasedMEI(true);
    return meioutput.GetOutput();
}

//////////////////////////////
//
// HumdrumInput::setLocationId -- use the file location of the item
//    for the ID.
//     default value: subtoken = -1 (don't display subtoken number);
//

void HumdrumInput::setLocationId(Object *object, hum::HTp token, int subtoken)
{
    object->SetID(getLocationId(object, token, subtoken));
}

void HumdrumInput::setLocationId(Object *object, int lineindex, int fieldindex, int subtokenindex)
{
    object->SetID(getLocationId(object, lineindex, fieldindex, subtokenindex));
}

///////////////////////////////////
//
// HumdrumInput::getLocationId --
//

std::string HumdrumInput::getLocationId(Object *object, hum::HTp token, int subtoken)
{
    int line = token->getLineIndex() + 1;
    int field = token->getFieldIndex() + 1;
    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    id += "-L" + to_string(line);
    id += "F" + to_string(field);
    if (subtoken >= 0) {
        id += "S" + to_string(subtoken + 1);
    }
    return id;
}

std::string HumdrumInput::getLocationId(const std::string &prefix, hum::HTp token, int subtoken)
{
    int line = token->getLineIndex() + 1;
    int field = token->getFieldIndex() + 1;
    std::string id = prefix;
    id += "-L" + to_string(line);
    id += "F" + to_string(field);
    if (subtoken >= 0) {
        id += "S" + to_string(subtoken + 1);
    }
    return id;
}

std::string HumdrumInput::getLocationId(Object *object, int lineindex, int fieldindex, int subtokenindex)
{
    int line = lineindex + 1;
    int field = fieldindex + 1;
    int subtoken = subtokenindex + 1;
    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    if (line > 0) {
        id += "-L" + to_string(line);
    }
    if (field > 0) {
        id += "F" + to_string(field);
    }
    if (subtoken > 0) {
        id += "S" + to_string(subtoken);
    }
    return id;
}

std::string HumdrumInput::getLocationId(const std::string &prefix, int lineindex, int fieldindex, int subtokenindex)
{
    int line = lineindex + 1;
    int field = fieldindex + 1;
    int subtoken = subtokenindex + 1;
    std::string id = prefix;
    if (line > 0) {
        id += "-L" + to_string(line);
    }
    if (field > 0) {
        id += "F" + to_string(field);
    }
    if (subtoken > 0) {
        id += "S" + to_string(subtoken);
    }
    return id;
}

//////////////////////////////
//
// HumdrumInput::setLocationIdNSuffix --
//

void HumdrumInput::setLocationIdNSuffix(Object *object, hum::HTp token, int number)
{
    int line = token->getLineIndex() + 1;
    int field = token->getFieldIndex() + 1;
    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    id += "-L" + to_string(line);
    id += "F" + to_string(field);
    id += "N" + to_string(number);
    object->SetID(id);
}

/////////////////////////////
//
// HumdrumInput::checkBeamWith -- If all notes of a beam are not on the home staff,
//    then use beam@beam.width="below|above" to move it to the adjacent staff (it
//    will not be moved there automatically by verovio).
//

void HumdrumInput::checkBeamWith(
    Beam *beam, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs, std::vector<hum::HTp> &layerdata, int startindex)
{
    if (!(m_signifiers.above || m_signifiers.below)) {
        return;
    }
    hum::HumRegex hre;
    int targetEnd = tgs[startindex].beamstart;
    bool isAbove = true;
    bool isBelow = true;
    bool foundAbove = false;
    bool foundBelow = false;
    std::string aboveSearch = "[A-Ga-gn#-][XxYy]?";
    std::string belowSearch = "[A-Ga-gn#-][XxYy]?";
    aboveSearch += m_signifiers.above;
    belowSearch += m_signifiers.below;
    for (int i = startindex; i < (int)layerdata.size(); ++i) {
        if (!(layerdata[i]->isNote() || layerdata[i]->isRest())) {
            continue;
        }
        if (isAbove) {
            if (hre.search(layerdata[i], aboveSearch)) {
                foundAbove = true;
            }
            else {
                foundAbove = false;
            }
        }
        if (isBelow) {
            if (hre.search(layerdata[i], belowSearch)) {
                foundBelow = true;
            }
            else {
                foundBelow = false;
            }
        }
        isAbove &= foundAbove;
        isBelow &= foundBelow;
        if (!(isAbove || isBelow)) {
            break;
        }
        if (tgs[i].beamend == targetEnd) {
            break;
        }
    }
    if (!(isAbove || isBelow)) {
        return;
    }
    if (isAbove && isBelow) {
        // something strange happened
    }
    if (isAbove) {
        beam->SetBeamWith(NEIGHBORINGLAYER_above);
    }
    if (isBelow) {
        beam->SetBeamWith(NEIGHBORINGLAYER_below);
    }
}

/////////////////////////////
//
// HumdrumInput::setBeamLocationId --
//

void HumdrumInput::setBeamLocationId(Object *object, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs,
    std::vector<hum::HTp> &layerdata, int startindex)
{
    int startnum = tgs[startindex].beamstart;
    hum::HTp starttoken = layerdata[startindex];
    int startline = starttoken->getLineNumber();
    int startfield = starttoken->getFieldNumber();

    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);

    // int endnum = -1;
    int endindex = -1;
    for (int i = startindex + 1; i < (int)tgs.size(); ++i) {
        if (tgs[i].beamend == startnum) {
            // endnum = startnum;
            endindex = i;
            break;
        }
    }

    if (endindex > 0) {
        hum::HTp endtoken = layerdata[endindex];
        int endline = endtoken->getLineNumber();
        int endfield = endtoken->getFieldNumber();
        id += "-L" + to_string(endline);
        id += "F" + to_string(endfield);
    }

    object->SetID(id);
}

/////////////////////////////
//
// HumdrumInput::setTupletLocationId --
//

void HumdrumInput::setTupletLocationId(Object *object, const std::vector<humaux::HumdrumBeamAndTuplet> &tgs,
    std::vector<hum::HTp> &layerdata, int startindex)
{
    int startnum = tgs[startindex].tupletstart;
    hum::HTp starttoken = layerdata[startindex];
    int startline = starttoken->getLineNumber();
    int startfield = starttoken->getFieldNumber();

    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);

    // int endnum = -1;
    int endindex = -1;
    for (int i = startindex + 1; i < (int)tgs.size(); ++i) {
        if (tgs[i].tupletend == startnum) {
            // endnum = startnum;
            endindex = i;
            break;
        }
    }

    if (endindex > 0) {
        hum::HTp endtoken = layerdata[endindex];
        int endline = endtoken->getLineNumber();
        int endfield = endtoken->getFieldNumber();
        id += "-L" + to_string(endline);
        id += "F" + to_string(endfield);
    }

    object->SetID(id);
}

/////////////////////////////
//
// HumdrumInput::setTieLocationId --
//

void HumdrumInput::setTieLocationId(Object *object, hum::HTp tiestart, int sindex, hum::HTp tieend, int eindex)
{

    int startline = 0;
    int startfield = 0;
    int endline = 0;
    int endfield = 0;

    if (tiestart) {
        startline = tiestart->getLineNumber();
        startfield = tiestart->getFieldNumber();
    }
    if (tieend) {
        endline = tieend->getLineNumber();
        endfield = tieend->getFieldNumber();
    }

    std::string id = object->GetClassName();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);

    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);
    if (sindex >= 0) {
        id += "S" + to_string(sindex + 1);
    }

    id += "-L" + to_string(endline);
    id += "F" + to_string(endfield);
    if (eindex >= 0) {
        id += "S" + to_string(eindex + 1);
    }

    object->SetID(id);
}

/////////////////////////////
//
// HumdrumInput::setSlurLocationId -- Not dealing with cross-over slurs yet,
// such as &( and &).
//

void HumdrumInput::setSlurLocationId(
    Object *object, hum::HTp slurstart, hum::HTp slurend, int slurstartnumber, const std::string &prefix)
{
    int startline = slurstart->getLineNumber();
    int startfield = slurstart->getFieldNumber();
    std::string id;
    if (prefix.empty()) {
        id = object->GetClassName();
    }
    else {
        id = prefix;
    }
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    id += "-L" + to_string(startline);
    id += "F" + to_string(startfield);
    int startcount = slurstart->getValueInt("auto", "slurStartCount");
    int endcount = slurend->getValueInt("auto", "slurEndCount");

    std::string tag = "slurEndNumber";
    if (slurstartnumber > 1) {
        tag += to_string(slurstartnumber);
    }
    int slurendnumber = slurstart->getValueInt("auto", tag);

    if (startcount > 1) {
        id += "N";
        id += to_string(slurstartnumber);
    }

    int endline = slurend->getLineNumber();
    int endfield = slurend->getFieldNumber();

    id += "-L";
    id += to_string(endline);
    id += "F";
    id += to_string(endfield);

    if (endcount > 1) {
        id += "N";
        id += to_string(slurendnumber);
    }

    object->SetID(id);
}

//////////////////////////////
//
// HumdrumInput::parseSignifiers -- search for !!!RDF records which
//     define special purpose notational features in Humdrum data spines.
//     Currently recognized ones are:
// !!!RDF**kern: i = no stem
// only single-character signifiers are allowed (could be made a string)
//

void HumdrumInput::parseSignifiers(hum::HumdrumFile &infile)
{
    hum::HumRegex hre;
    std::vector<hum::HumdrumLine *> refs = infile.getReferenceRecords();
    for (int i = 0; i < (int)refs.size(); ++i) {
        std::string key = refs[i]->getReferenceKey();
        std::string value = refs[i]->getReferenceValue();
        auto equals = value.substr(0, 8).find('=');
        std::string afterequals;
        if (equals != std::string::npos) {
            afterequals = value.substr(equals + 1);
        }
        else {
            afterequals = value;
        }

        if ((equals == std::string::npos) && (key == "RDF**kern")) {
            // meta signifiers (no actual signifier)

            // colored spaces (meta signifiers)
            // !!!RDF**kern: show spaces color=hotpink
            // !!!RDF**kern: show invisible rests color=chartreuse
            // !!!RDF**kern: show implicit spaces color=purple
            // !!!RDF**kern: show recip spaces color=royalblue
            if (value.find("show space") != std::string::npos) {
                if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                    m_signifiers.space_color = hre.getMatch(1);
                }
                else {
                    m_signifiers.space_color = "hotpink";
                }
            }
            if (value.find("show invisible rest") != std::string::npos) {
                if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                    m_signifiers.irest_color = hre.getMatch(1);
                }
                else {
                    m_signifiers.irest_color = "chartreuse";
                }
            }
            if (value.find("show implicit space") != std::string::npos) {
                if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                    m_signifiers.ispace_color = hre.getMatch(1);
                }
                else {
                    m_signifiers.ispace_color = "blueviolet";
                }
            }
            if (value.find("show recip space") != std::string::npos) {
                if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                    m_signifiers.rspace_color = hre.getMatch(1);
                }
                else {
                    m_signifiers.rspace_color = "royalblue";
                }
            }

            continue;
        }

        char signifier = 0;
        for (int j = 0; j < (int)equals; ++j) {
            if (isspace(value[j])) {
                continue;
            }
            signifier = value[j];
            break;
        }
        if (!signifier) {
            continue;
        }
        // check for known signifier meanings:

        if (((key == "RDF**silbe") || (key == "RDF**text")) && hre.search(value, "marked text|matched text")) {
            // for **text and **silbe (and text-like/silbe-like)
            m_signifiers.textmark.push_back(signifier);
            if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                m_signifiers.textcolor.push_back(hre.getMatch(1));
            }
            else {
                m_signifiers.textcolor.push_back("red");
            }
        }
        else if (key == "RDF**mens") {
            // colored mensural music
            // !!!RDF**mens: Y = marked note, color="#ff0000"
            // !!!RDF**mens: Z = matched note, color=blue
            if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                m_signifiers.mens_mark.push_back(signifier);
                m_signifiers.mens_mcolor.push_back(hre.getMatch(1));
                if (hre.search(value, "text\\s*=\\s*\"?([^\"]+)\"?")) {
                    m_signifiers.mens_markdir.push_back(hre.getMatch(1));
                }
                else {
                    m_signifiers.mens_markdir.push_back("");
                }
            }
            else if (hre.search(value, "marked note|matched note")) {
                m_signifiers.mens_mark.push_back(signifier);
                m_signifiers.mens_mcolor.push_back("red");
                if (hre.search(value, "text\\s*=\\s*\"?([^\"]+)\"?")) {
                    m_signifiers.mens_markdir.push_back(hre.getMatch(1));
                }
                else {
                    m_signifiers.mens_markdir.push_back("");
                }
            }
        }
        else if (key == "RDF**dynam") {
            if (signifier == '>') {
                if (hre.search(afterequals, "^\\s*\"\\s*([^\"]+)\\s*\"")) {
                    m_signifiers.decresctext = hre.getMatch(1);
                }
                else if (hre.search(afterequals, "^\\s*([^\\s]+)")) {
                    m_signifiers.decresctext = hre.getMatch(1);
                }
                if (hre.search(afterequals, "fontstyle\\s*=\\s*\"?\\s*([^\"\\s]+)\\s*\"?")) {
                    m_signifiers.decrescfontstyle = hre.getMatch(1);
                    if (m_signifiers.crescfontstyle == "italic") {
                        // default is italic for verovio, so suppress:
                        m_signifiers.crescfontstyle = "";
                    }
                }
            }
            else if (signifier == '<') {
                if (hre.search(afterequals, "^\\s*\"\\s*([^\"]+)\\s*\"")) {
                    m_signifiers.cresctext = hre.getMatch(1);
                }
                else if (hre.search(afterequals, "^\\s*([^\\s]+)")) {
                    m_signifiers.cresctext = hre.getMatch(1);
                }
                if (hre.search(afterequals, "fontstyle\\s*=\\s*\"?\\s*([^\"\\s]+)\\s*\"?")) {
                    m_signifiers.crescfontstyle = hre.getMatch(1);
                    if (m_signifiers.crescfontstyle == "italic") {
                        // default is italic for verovio, so suppress:
                        m_signifiers.crescfontstyle = "";
                    }
                }
            }
        }

        if (key != "RDF**kern") {
            continue;
        }

        // stemless note:
        // !!!RDF**kern: i = no stem
        if (value.find("no stem", equals) != std::string::npos) {
            m_signifiers.nostem = signifier;
        }

        // cue-sized note:
        // !!!RDF**kern: i = cue size
        if (value.find("cue size", equals) != std::string::npos) {
            m_signifiers.cuesize = signifier;
        }

        // hairpin accents:
        // !!!RDF**kern: i = hairpin accent
        if (value.find("hairpin accent", equals) != std::string::npos) {
            m_signifiers.hairpinAccent = signifier;
        }

        // vertical strokes:
        // !!!RDF**kern: | = vertical stroke
        if (value.find("vertical stroke", equals) != std::string::npos) {
            m_signifiers.verticalStroke = signifier;
        }

        // left-hand pizzicatos:
        // !!!RDF**kern: + = l.h. pizz.
        if (value.find("l.h. pizz", equals) != std::string::npos) {
            m_signifiers.lhpizz = signifier;
        }
        else if (value.find("left hand pizz", equals) != std::string::npos) {
            m_signifiers.lhpizz = signifier;
        }
        else if (value.find("left-hand pizz", equals) != std::string::npos) {
            m_signifiers.lhpizz = signifier;
        }
        else if (value.find("lefthand pizz", equals) != std::string::npos) {
            m_signifiers.lhpizz = signifier;
        }

        // tremolo slashes on stem
        if (value.find("tremolo", equals) != std::string::npos) {
            m_signifiers.tremolo = signifier;
        }

        // terminal longs
        // !!!RDF**kern: i = terminal long
        if (value.find("terminal long", equals) != std::string::npos) {
            m_signifiers.terminallong = signifier;
        }
        else if (value.find("long note", equals) != std::string::npos) {
            m_signifiers.terminallong = signifier;
        }

        // terminal breves
        // !!!RDF**kern: i = terminal breve
        if (value.find("terminal breve", equals) != std::string::npos) {
            m_signifiers.terminalbreve = signifier;
        }
        else if (value.find("breve note", equals) != std::string::npos) {
            m_signifiers.terminalbreve = signifier;
        }

        // slur directions
        if (value.find("above", equals) != std::string::npos) {
            m_signifiers.above = signifier;
        }
        if (value.find("below", equals) != std::string::npos) {
            m_signifiers.below = signifier;
        }

        // editorial accidentals:
        if (value.find("editorial accidental", equals) != std::string::npos) {
            m_signifiers.editacc.push_back(signifier);
            if (value.find("brack") != std::string::npos) {
                if (value.find("up") != std::string::npos) {
                    m_signifiers.edittype.push_back("brack-up");
                }
                else {
                    m_signifiers.edittype.push_back("brack");
                }
            }
            else if (value.find("paren") != std::string::npos) {
                if (value.find("up") != std::string::npos) {
                    m_signifiers.edittype.push_back("paren-up");
                }
                else {
                    m_signifiers.edittype.push_back("paren");
                }
            }
            else if (value.find("none") != std::string::npos) {
                m_signifiers.edittype.push_back("none");
            }
            else {
                m_signifiers.edittype.push_back("");
            }
        }

        else if (hre.search(value, "phrase")) {
            // default phrase styling
            if (value.find("none") != std::string::npos) {
                m_signifiers.phrase_style = "none";
            }
            else if (value.find("brack") != std::string::npos) {
                m_signifiers.phrase_style = "brack";
            }
            else if (value.find("dot") != std::string::npos) {
                m_signifiers.phrase_style = "dot";
            }
            else if (value.find("dash") != std::string::npos) {
                m_signifiers.phrase_style = "dash";
            }
            if (value.find("slur") != std::string::npos) {
                m_signifiers.phrase_slur = "slur";
            }
            if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
                m_signifiers.phrase_color = hre.getMatch(1);
            }
        }

        // colored notes
        // !!!RDF**kern: i = marked note, color="#ff0000"
        // !!!RDF**kern: i = matched note, color=blue
        else if (hre.search(value, "color\\s*=\\s*\"?([^\"\\s]+)\"?")) {
            m_signifiers.mark.push_back(signifier);
            m_signifiers.mcolor.push_back(hre.getMatch(1));
            if (hre.search(value, "text\\s*=\\s*\"?([^\"]+)\"?")) {
                m_signifiers.markdir.push_back(hre.getMatch(1));
            }
            else {
                m_signifiers.markdir.push_back("");
            }
        }
        else if (hre.search(value, "marked note|matched note")) {
            m_signifiers.mark.push_back(signifier);
            m_signifiers.mcolor.push_back("red");
            if (hre.search(value, "text\\s*=\\s*\"?([^\"]+)\"?")) {
                m_signifiers.markdir.push_back(hre.getMatch(1));
            }
            else {
                m_signifiers.markdir.push_back("");
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeBreaks -- Returns true if there are page or
//   system breaks in the data.
//

bool HumdrumInput::analyzeBreaks(hum::HumdrumFile &infile)
{

    // check for informal breaking markers such as:
    // !!pagebreak:original
    // !!linebreak:original
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isGlobalComment()) {
            continue;
        }
        hum::HTp token = infile.token(i, 0);
        if (token->compare(0, 12, "!!pagebreak:") == 0) {
            return true;
        }
        if (token->compare(0, 12, "!!linebreak:") == 0) {
            return true;
        }
    }

    // check for formal breaking markers such as:
    // !!LO:PB:g=original
    // !!LO:LB:g=original
    // !LO:PB:g=original
    // !LO:LB:g=original
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isComment()) {
            continue;
        }
        hum::HTp token = infile.token(i, 0);
        if (token->find("!LO:LB") != std::string::npos) {
            return true;
        }
        if (token->find("!LO:PB") != std::string::npos) {
            return true;
        }
    }

    return false;
}

//////////////////////////////
//
// HumdrumInput::analyzeMultiRest --
//

std::vector<int> HumdrumInput::analyzeMultiRest(hum::HumdrumFile &infile)
{

    if (m_mens) {
        // Do not calculate multirests for mensural music
        std::vector<int> output(infile.getLineCount(), 0);
        return output;
    }

    std::vector<int> barindex(1, 0); // line number of barline
    std::vector<int> datacount(1, 0);
    std::vector<int> dataline(1, 0);
    std::vector<hum::HumNum> bardur(1, 0);

    // count number of non-null data lines for each measure.
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (infile[i].isData()) {
            if (!infile[i].isAllNull()) {
                datacount.back() = datacount.back() + 1;
                if (datacount.back() == 1) {
                    dataline.back() = i;
                }
            }
            else {
                // line is all null tokens.
            }
        }
        if (!infile[i].isBarline()) {
            continue;
        }
        barindex.push_back(i);
        datacount.push_back(0);
        dataline.push_back(0);
        bardur.push_back(infile[i].getDurationToBarline());
    }

    // check to see if measures with single data item is a rest.
    std::vector<int> wholerest(barindex.size(), 0);
    std::vector<int> textrest(barindex.size(), 0);
    bool restQ;
    int line;
    for (int i = 0; i < (int)barindex.size(); ++i) {
        if (datacount[i] == 1) {
            restQ = true;
            line = dataline[i];
            for (int j = 0; j < infile[line].getFieldCount(); ++j) {
                if (!infile.token(line, j)->isKernLike()) {
                    continue;
                }
                if (!infile.token(line, j)->isRest()) {
                    restQ = false;
                    break;
                }
            }
            if (restQ) {
                wholerest[i] = 1;
            }
        }
    }

    // remove cases where **mxhm text is present at the same
    // time as the whole-measure rest.  There will still be cases
    // where there is a chord/dynamic which does not start
    // at the same time as the rest that need to be accounted for.
    for (int i = 0; i < (int)wholerest.size(); ++i) {
        if (wholerest[i] != 1) {
            continue;
        }
        bool hasitem = false;
        line = dataline[i];
        for (int j = 0; j < infile[line].getFieldCount(); ++j) {
            hum::HTp tok = infile.token(line, j);
            if (tok->isNull()) {
                continue;
            }
            if (tok->isDataType("**mxhm")) {
                hasitem = true;
                break;
            }
            else if (tok->isDataType("**dynam")) {
                hasitem = true;
                break;
            }
            else if (tok->isDataTypeLike("**text")) {
                hasitem = true;
                break;
            }
        }
        if (hasitem) {
            wholerest[i] = 0;
            textrest[i] = 1;
        }
    }

    // remove cases where there is text attached to the whole-measure rest
    for (int i = 0; i < (int)wholerest.size(); ++i) {
        if (wholerest[i] != 1) {
            continue;
        }
        bool hastext = false;
        line = dataline[i];
        for (int j = 0; j < infile[line].getFieldCount(); ++j) {
            hum::HTp tok = infile.token(line, j);
            if (tok->isNull()) {
                continue;
            }
            if (!tok->isKernLike()) {
                continue;
            }
            std::string text = tok->getLayoutParameter("TX", "t");
            if (!text.empty()) {
                hastext = true;
                break;
            }
        }
        if (hastext) {
            wholerest[i] = 0;
            textrest[i] = 1;
        }
    }

    // group sequences of whole-meaure rests
    for (int i = (int)wholerest.size() - 2; i >= 0; i--) {
        if (bardur[i] != bardur[i + 1]) {
            continue;
        }
        if (regex_search(*infile.token(barindex[i + 1], 0), regex("[^=0-9]"))) {
            continue;
        }
        if (wholerest[i] && wholerest[i + 1]) {
            wholerest[i] += wholerest[i + 1];
            wholerest[i + 1] = -1;
        }
    }

    // Expand backwards to include a whole-measure rest with a
    // measure that has text.
    for (int i = 0; i < (int)wholerest.size() - 1; ++i) {
        if (bardur[i] != bardur[i + 1]) {
            continue;
        }
        if ((textrest[i] == 1) && (wholerest[i + 1] >= 1)) {
            wholerest[i] = wholerest[i + 1] + 1;
            wholerest[i + 1] = -1;
        }
    }

    std::vector<int> output(infile.getLineCount(), 0);
    for (int i = 0; i < (int)wholerest.size(); ++i) {
        output[dataline[i]] = wholerest[i];
    }
    for (int i = infile.getLineCount() - 2; i >= 0; i--) {
        if (!infile[i + 1].isBarline()) {
            if (output[i + 1] != 0) {
                output[i] = output[i + 1];
            }
        }
    }

    // for (int i = 0; i < infile.getLineCount(); ++i) {
    //    cout << infile[i] << "\t" << output[i] << "\n";
    //}
    // Example analysis, with measure 4 staring a rest with num="6".
    // Measures 5-9 marked as whole-measure rests which will be merged into
    // the multi rest.
    //    **kern    **kern    0
    //    *M4/4    *M4/4    0
    //    =1-    =1-    0
    //    1c    1d    0
    //    =2    =2    0
    //    1d    1g    0
    //    =3    =3    0
    //    1r    1e    0
    //    =4    =4    6
    //    1r    1r    6
    //    =5    =5    -1
    //    1r    1r    -1
    //    =6    =6    -1
    //    1r    1r    -1
    //    =7    =7    -1
    //    1r    1r    -1
    //    =8    =8    -1
    //    1r    1r    -1
    //    =9    =9    -1
    //    1r    1r    -1
    //    =10    =10    0
    //    1f    1r    0
    //    =11    =11    0
    //    1g    1g    0
    //    =12    =12    0
    //    1a    1g    0
    //    ==    ==    0
    //    *-    *-    0

    if (!barindex.empty()) {
        int firstbar = barindex[0];
        if ((firstbar == 0) && (barindex.size() >= 2)) {
            firstbar = barindex[1];
        }
        hum::HumNum bardur = infile[firstbar].getDurationFromStart();
        if (bardur == 0) {
            // Extend first non-zero number in list backwards to start of output.
            // This allows a multibar rest at the start of the music.
            for (int i = 0; i < firstbar; ++i) {
                output[i] = output[firstbar];
            }
        }
    }

    // Mark empty and null lines inside of multi-measure rest regions:
    for (int i = 0; i < (int)output.size(); ++i) {
        if (!output[i]) {
            continue;
        }
        if (!infile[i].isBarline()) {
            continue;
        }
        for (int j = i + 1; j < infile.getLineCount(); j++) {
            if (infile[j].isBarline()) {
                i = j - 1;
                break;
            }
            if (*infile.token(j, 0) == "*-") {
                break;
            }
            output[j] = -1;
        }
    }

    return output;
}

//////////////////////////////
//
// HumdrumInput::prepareSections --
//

void HumdrumInput::prepareSections()
{
    std::vector<hum::HTp> &sectionlabels = m_sectionlabels;
    std::vector<hum::HTp> &numberlesslabels = m_numberlesslabels;
    hum::HumdrumFile &infile = m_infiles[0];

    sectionlabels.resize(infile.getLineCount());
    numberlesslabels.resize(infile.getLineCount());
    for (int i = 0; i < (int)sectionlabels.size(); ++i) {
        sectionlabels[i] = NULL;
        numberlesslabels[i] = NULL;
    }
    hum::HTp secname = NULL;
    hum::HTp nonumname = NULL;

    for (int i = 0; i < infile.getLineCount(); ++i) {
        sectionlabels[i] = secname;
        numberlesslabels[i] = nonumname;
        if (!infile[i].isInterpretation()) {
            continue;
        }
        if (infile.token(i, 0)->compare(0, 2, "*>") != 0) {
            continue;
        }
        if (infile.token(i, 0)->find("[") != std::string::npos) {
            // ignore expansion lists
            continue;
        }

        //        std::smatch matches;
        //        if (regex_search(*((string *)infile.token(i, 0)), matches, regex("(\\d+)$"))) {
        //            endnum = stoi(matches[1]);
        //            ending[i] = endnum;
        //        }
        //        else {
        //            endnum = 0;
        //            ending[i] = endnum;
        //        }

        secname = infile.token(i, 0);
        sectionlabels[i] = secname;
        for (int j = i - 1; j >= 0; j--) {
            if (infile[j].isData()) {
                break;
            }
            sectionlabels[j] = sectionlabels[i];
        }

        if (!isdigit(secname->back())) {
            nonumname = secname;
            sectionlabels[i] = nonumname;
            for (int j = i - 1; j >= 0; j--) {
                if (infile[j].isData()) {
                    break;
                }
                numberlesslabels[j] = numberlesslabels[i];
            }
        }
    }

    for (int i = (int)numberlesslabels.size() - 2; i >= 0; i--) {
        if (numberlesslabels[i] == NULL) {
            if (numberlesslabels[i + 1]) {
                numberlesslabels[i] = numberlesslabels[i + 1];
            }
        }
    }
}

//////////////////////////////
//
// HumdrumInput::checkForColorSpine -- Look for a **color spine in the input
// data.
//

void HumdrumInput::checkForColorSpine(hum::HumdrumFile &infile)
{
    std::vector<hum::HTp> colorspines;
    infile.getSpineStartList(colorspines, "**color");
    m_has_color_spine = colorspines.empty() ? false : true;
}

//////////////////////////////
//
// storeExpansionLists --
//

void HumdrumInput::storeExpansionLists(Section *section, hum::HTp starting)
{
    hum::HTp current = starting;
    std::vector<hum::HTp> expansions;

    while (current) {
        if (current->isData()) {
            // only look for expansion lists before first data line
            break;
        }
        if (!current->isInterpretation()) {
            current = current->getNextToken();
            continue;
        }
        if (current->compare(0, 2, "*>") != 0) {
            current = current->getNextToken();
            continue;
        }
        if (current->find("[") == std::string::npos) {
            current = current->getNextToken();
            continue;
        }
        expansions.push_back(current);
        current = current->getNextToken();
    }

    if (expansions.empty()) {
        return;
    }
    else if (expansions.size() == 1) {
        storeExpansionList(section, expansions.at(0));
    }
    else {
        storeExpansionListsInChoice(section, expansions);
    }
}

//////////////////////////////
//
// HumdrumInput::storeExpansionListsInChoice --
//

void HumdrumInput::storeExpansionListsInChoice(Section *section, std::vector<hum::HTp> &expansions)
{
    Choice *choice = new Choice();
    section->AddChild(choice);

    // Extract the variant labels:
    std::vector<std::string> labels(expansions.size());
    hum::HumRegex hre;
    for (int i = 0; i < (int)expansions.size(); ++i) {
        if (hre.search(expansions.at(i), "\\*>([^[]+)[[]")) {
            labels.at(i) = hre.getMatch(1);
        }
    }

    // Store the primary expansion:
    for (int i = 0; i < (int)labels.size(); ++i) {
        if (labels.at(i).empty()) {
            Orig *orig = new Orig();
            choice->AddChild(orig);
            storeExpansionList(orig, expansions.at(i));
            break; // if there is more than one primary,
                   // the secondary ones will be ignored.
        }
    }

    // Store the secondary expansions:
    for (int i = 0; i < (int)labels.size(); ++i) {
        if (labels.at(i).empty()) {
            continue;
        }
        Reg *reg = new Reg();
        choice->AddChild(reg);
        reg->SetType(labels.at(i));
        storeExpansionList(reg, expansions.at(i));
    }
}

//////////////////////////////
//
// HumdrumInput::storeExpansionList --
//

template <class ELEMENT> void HumdrumInput::storeExpansionList(ELEMENT *parent, hum::HTp etok)
{
    std::string expansion = *etok;
    std::string variant;
    int startindex = -1;
    for (int i = 2; i < (int)expansion.size(); ++i) {
        if (expansion[i] == '[') {
            startindex = i + 1;
            break;
        }
        variant += expansion[i];
    }
    if (startindex < 0) {
        return;
    }
    std::vector<std::string> labels(1);
    for (int i = startindex; i < (int)expansion.size(); ++i) {
        if (isspace(expansion[i])) {
            continue;
        }
        else if (expansion[i] == '"') {
            // invalid character
            continue;
        }
        else if (expansion[i] == '\'') {
            // invalid character
            continue;
        }
        else if (expansion[i] == ',') {
            if (!labels.back().empty()) {
                // avoid syntax error from a null label.
                labels.push_back("");
            }
        }
        else if (expansion[i] == ']') {
            break;
        }
        else {
            labels.back() += expansion[i];
        }
    }

    if (labels.empty()) {
        return;
    }
    if ((labels.size() == 1) && labels[0].empty()) {
        return;
    }

    Expansion *exp = new Expansion();
    exp->SetID(getLocationId(exp, etok, -1));
    parent->AddChild(exp);
    if (!variant.empty()) {
        exp->SetType(variant);
    }

    for (int i = 0; i < (int)labels.size(); ++i) {
        std::string ref = "#label-" + labels[i];
        exp->AddRefAllowDuplicate(ref);
    }
}

//////////////////////////////
//
// HumdrumInput::hideTerminalBarlines -- Barlines during a terminal long
//    notes are made invisible.
//

void HumdrumInput::hideTerminalBarlines(hum::HumdrumFile &infile)
{
    for (int i = 0; i < infile.getStrandCount(); ++i) {
        hum::HTp stok = infile.getStrandStart(i);
        if (!stok->isKernLike()) {
            continue;
        }
        hum::HTp etok = infile.getStrandEnd(i);
        hum::HTp tok = stok;
        while (tok && (tok != etok)) {
            if (!tok->isData()) {
                tok = tok->getNextToken();
                continue;
            }
            if (tok->isNull()) {
                tok = tok->getNextToken();
                continue;
            }
            if (tok->find('[') == std::string::npos) {
                tok = tok->getNextToken();
                continue;
            }
            if (m_signifiers.terminallong && (tok->find(m_signifiers.terminallong) == std::string::npos)) {
                tok = tok->getNextToken();
                continue;
            }
            else if (m_signifiers.terminalbreve && (tok->find(m_signifiers.terminalbreve) == std::string::npos)) {
                tok = tok->getNextToken();
                continue;
            }
            hideBarlinesInTiedGroup(tok);
            tok = tok->getNextToken();
        }
    }
}

//////////////////////////////
//
// HumdrumInput::hideBarlinesInTiedGroup -- Barlines found between the starting
//    note and the end of the tied group are made invisible.
//

void HumdrumInput::hideBarlinesInTiedGroup(hum::HTp startnote)
{
    hum::HTp current = startnote;
    if (!startnote) {
        return;
    }
    if (startnote->find('[') == std::string::npos) {
        return;
    }
    while (current) {
        if (current->isBarline()) {
            std::string text = *current;
            text += "-";
            current->setText(text);
        }
        else if (current->isData() && (current->find(']') != std::string::npos)) {
            if (current->find(';') != std::string::npos) {
                if (startnote->find(';') == std::string::npos) {
                    std::string text = *startnote;
                    text += ';';
                    startnote->setText(text);
                }
            }
            break;
        }
        current = current->getNextToken();
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeClefNulls -- Mark all null interpretations
//    that are in the same track as a clef interpretation.
//

void HumdrumInput::analyzeClefNulls(hum::HumdrumFile &infile)
{
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isInterpretation()) {
            continue;
        }
        for (int j = 0; j < infile[i].getFieldCount(); j++) {
            hum::HTp token = infile[i].token(j);
            if (!token->isKernLike()) {
                continue;
            }
            if (!token->isClef()) {
                continue;
            }
            markAdjacentNullsWithClef(token);
        }
    }
}

//////////////////////////////
//
// HumdrumInput::markAdjacentNullsWithClef -- Input is a clef token,
//     and all null interpretations in the same spine will be marked
//     as being the same clef, since verovio/MEI requires clef changes
//     to be present in all layers.
//

void HumdrumInput::markAdjacentNullsWithClef(hum::HTp clef)
{
    int ctrack = clef->getTrack();
    int track;

    hum::HTp current = clef->getNextFieldToken();
    while (current) {
        track = current->getTrack();
        if (track != ctrack) {
            break;
        }
        if (*current == "*") {
            current->setValue("auto", "clef", *clef);
        }
        current = current->getNextFieldToken();
    }

    current = clef->getPreviousFieldToken();
    while (current) {
        track = current->getTrack();
        if (track != ctrack) {
            break;
        }
        if (*current == "*") {
            current->setValue("auto", "clef", *clef);
        }
        current = current->getPreviousFieldToken();
    }
}

//////////////////////////////
//
// HumdrumInput::markOtherClefsAsChange -- There is a case
//     where spine splits at the start of the music miss a clef
//     change that needs to be added to a secondary layer.  This
//     function will mark the secondary clefs so that they will
//     be converted as clef changes.
//

void HumdrumInput::markOtherClefsAsChange(hum::HTp clef)
{

    int ctrack = clef->getTrack();
    int track;

    hum::HTp current = clef->getNextFieldToken();
    while (current) {
        track = current->getTrack();
        if (track != ctrack) {
            break;
        }
        current->setValue("auto", "clefChange", 1);
        current = current->getNextFieldToken();
    }

    current = clef->getPreviousFieldToken();
    while (current) {
        track = current->getTrack();
        if (track != ctrack) {
            break;
        }
        current->setValue("auto", "clefChange", 1);
        current = current->getPreviousFieldToken();
    }
}

//////////////////////////////
//
// HumdrumInput::importVerovioOptions -- Set any options coming from the toolkit interface here.
//

void HumdrumInput::importVerovioOptions(Doc *doc)
{
    m_humtype = doc->GetOptions()->m_humType.GetValue();
}

//////////////////////////////
//
// HumdrumInput::finalizeDocument -- For use when loaded a Humdrum file directly
//     into Verovio rather than indirectly through MEIInput class.  These functions
//     are taken from MEIInput::ReadDoc().
//

void HumdrumInput::finalizeDocument(Doc *doc)
{
    doc->ExpandExpansions();
    doc->ConvertToPageBasedDoc();
    doc->ConvertMarkupDoc();

    if (m_mens) {
        doc->SetMensuralMusicOnly(true);
        doc->m_notationType = NOTATIONTYPE_mensural;
        doc->ConvertToCastOffMensuralDoc(true);
    }
}

//////////////////////////////
//
// HumdrumInput::analyzeVerseColor -- Calculate color of lyric text from *color:
//    interpretations.
//

void HumdrumInput::analyzeVerseColor(hum::HumdrumFile &infile)
{
    std::vector<hum::HTp> exinterps;
    infile.getSpineStartList(exinterps, "**text");
    for (int i = 0; i < (int)exinterps.size(); ++i) {
        analyzeVerseColor(exinterps[i]);
    }
}

void HumdrumInput::analyzeVerseColor(hum::HTp &token)
{
    // Only checking primary spine (no spine splits)
    hum::HTp current = token;
    std::string color = "";
    hum::HumRegex hre;
    while (current) {
        if (current->isInterpretation()) {
            if (hre.search(current, "^\\*color:\\s*([^\\s]+)")) {
                color = hre.getMatch(1);
                if (color == "black") {
                    color = "";
                }
                else if (color == "#000") {
                    color = "";
                }
                else if (color == "#000000") {
                    color = "";
                }
            }
            else if (hre.search(current, "^\\*color:")) {
                color = "";
            }
        }
        if (color.empty()) {
            current = current->getNextToken();
            continue;
        }
        if (!current->isData()) {
            current = current->getNextToken();
            continue;
        }
        if (current->isNull()) {
            current = current->getNextToken();
            continue;
        }
        std::string localColor = current->getLayoutParameter("LY", "color");
        if (!localColor.empty()) {
            if (localColor == "black") {
                localColor = "";
            }
            else if (localColor == "#000") {
                localColor = "";
            }
            else if (localColor == "#000000") {
                localColor = "";
            }
            if (!localColor.empty()) {
                current->setValue("auto", "color", localColor);
            }
        }
        else {
            current->setValue("auto", "color", color);
        }
        current = current->getNextToken();
        continue;
    }
}

//////////////////////////////
//
// HumdrumInput::getGlobalTempoScaling --
//

double HumdrumInput::getGlobalTempoScaling(hum::HumdrumFile &infile)
{
    double output = 1.0;
    hum::HumRegex hre;
    for (int i = 0; i < infile.getLineCount(); ++i) {
        if (!infile[i].isGlobalReference()) {
            continue;
        }
        hum::HTp token = infile.token(i, 0);
        if (token->compare(0, 17, "!!!tempo-scaling:") != 0) {
            continue;
        }
        std::string value = infile[i].getReferenceValue();
        if (value.size() == 0) {
            continue;
        }
        if (hre.search(value, "[+-]?(0?\\.?\\d+)")) {
            double number = hre.getMatchDouble(1);
            if (hre.search(value, "%")) {
                number = number / 100.0;
            }
            else if (number >= 10.0) {
                number = number / 100.0;
            }
            if (number > 0.0) {
                output *= number;
            }
        }
    }
    return output;
}

//////////////////////////////
//
// HumdrumInput::isTacet -- Returns true if *tacet interpretation
//     is found before the first data line.  Such a spine is then
//     ignored when printing the score (remove *tacet or change to
//     *Xtacet to display the tacet part).

bool HumdrumInput::isTacet(hum::HTp spinestart)
{
    hum::HTp current = spinestart->getNextToken();
    while (current) {
        if (current->isData()) {
            break;
        }
        if (*current == "*tacet") {
            return true;
        }
        current = current->getNextToken();
    }
    return false;
}

//////////////////////////////
//
// HumdrumInput::insertGlissandos --
//

void HumdrumInput::insertGlissandos(std::vector<hum::HTp> &tokens)
{
    for (int i = 0; i < (int)tokens.size(); i++) {
        hum::HTp nexttok = NULL;
        hum::HTp current = tokens[i]->getNextToken();
        while (current) {
            if (!current->isData()) {
                current = current->getNextToken();
                continue;
            }
            if (current->isNull()) {
                current = current->getNextToken();
                continue;
            }
            nexttok = current;
            break;
        }
        if (!nexttok) {
            continue;
        }
        createGlissando(tokens[i], nexttok);
    }
    tokens.clear();
}

//////////////////////////////
//
// HumdrumInput::createGlissando --
//

void HumdrumInput::createGlissando(hum::HTp glissStart, hum::HTp glissEnd)
{
    if (glissEnd->find('h') == std::string::npos) {
        return;
    }
    int staffnumber = m_currentstaff;

    std::vector<int> gstarts;
    std::vector<int> gends;
    std::vector<std::string> stoks = glissStart->getSubtokens();
    std::vector<std::string> etoks = glissEnd->getSubtokens();

    if (!glissStart->isChord()) {
        gstarts.push_back(0);
    }
    else {
        for (int i = 0; i < (int)stoks.size(); i++) {
            if (stoks[i].find("H") != std::string::npos) {
                gstarts.push_back(i);
            }
        }
    }

    if (!glissEnd->isChord()) {
        gends.push_back(0);
    }
    else {
        for (int i = 0; i < (int)etoks.size(); i++) {
            if (etoks[i].find("h") != std::string::npos) {
                gends.push_back(i);
            }
        }
    }

    int minsize = (int)gstarts.size();
    if (minsize > (int)gends.size()) {
        minsize = (int)gends.size();
    }

    for (int i = 0; i < minsize; i++) {
        std::string stok = stoks[gstarts[i]];
        std::string etok = etoks[gends[i]];

        Gliss *gliss = new Gliss();
        setStaff(gliss, staffnumber);
        if (stok.find("HH") != std::string::npos) {
            gliss->SetLform(LINEFORM_wavy);
        }
        std::string startid = getLocationId("note", glissStart);
        if (glissStart->isChord()) {
            startid += "S";
            startid += to_string(gstarts[i] + 1);
        }

        std::string endid = getLocationId("note", glissEnd);
        if (glissEnd->isChord()) {
            endid += "S";
            endid += to_string(gends[i] + 1);
        }

        gliss->SetStartid("#" + startid);
        gliss->SetEndid("#" + endid);
        std::string glissId = "gliss-L";
        glissId += to_string(glissStart->getLineNumber());
        glissId += "F";
        glissId += to_string(glissStart->getFieldNumber());
        if (glissStart->isChord()) {
            glissId += "S";
            glissId += to_string(i + 1);
        }
        gliss->SetID(glissId);
        m_measure->AddChild(gliss);
    }
}

#endif /* NO_HUMDRUM_SUPPORT */

} // namespace vrv

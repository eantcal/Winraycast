// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
//
// This file is part of the WinRayCast Application (a 3D Engine Demo).
// This program is free software; you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this program; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.


/* -------------------------------------------------------------------------- */

#include "WorldMap.h"

#include <fstream>

#include "mip_unicode.h"
#include "mip_tknzr_bldr.h"
#include "mip_esc_cnvrtr.h"

#include <iostream>


/* -------------------------------------------------------------------------- */

bool WorldMap::setMapInfo(const Cell* array, uint32_t rows, uint32_t cols)
{
    if (rows <= 0 || cols <= 0) {
        return false;
    }

    m_map.resize(rows);

    int i = 0;

    for (uint32_t r = 0; r < rows; ++r) {
        m_map[r].resize(cols);

        for (uint32_t c = 0; c < cols; ++c) {
            m_map[r][c] = array[i++];
        }
    }

    m_maxX = getCellDx() * getColCount();
    m_maxY = getCellDy() * getRowCount();

    return true; // success
}


/* -------------------------------------------------------------------------- */

bool WorldMap::load(const std::string& fileName)
{
    mip::tknzr_bldr_t bldr;

    bldr.def_atom(_T("{"));
    bldr.def_atom(_T("}"));
    bldr.def_atom(_T(","));

    bldr.def_sl_comment(_T("//"));
    bldr.def_sl_comment(_T("#"));
    bldr.def_blank(_T(" "));
    bldr.def_blank(_T("\r")); // treat \r like a blank
    bldr.def_blank(_T("\t"));

    bldr.def_eol(mip::base_tknzr_t::eol_t::LF); // linefeed is end of line marker

    bldr.def_string(_T('\"'), std::make_shared<mip::esc_cnvrtr_t>(_T('\\')));

    bldr.def_ml_comment(_T("/*"), _T("*/"));

    auto tknzr = bldr.build();

    mip::_ifstream is(fileName, std::ios::in | std::ios::binary);

    if (!is.is_open()) {
        return false;
    }

    enum state_t {
        ANY_KEY,
        MAP_BEGIN,
        MAP_VAL,
        TEXTURE_BEGIN,
        TEXTURE_KEY,
        TEXTURE_VALUE
    };

    state_t st = ANY_KEY;
    std::vector<uint64_t> mapValues;
    std::map<std::string, std::string> textureMap;
    std::string txtKey;

    int cols = -1;
    int rows = 0;
    int offset = 0;

    using tcl_t = mip::token_t::tcl_t;

    while (is.is_open() && !is.bad()) {
        auto tkn = tknzr->next(is);

        if (!tkn) {
            return false;
        }

        switch (tkn->type()) {
            case mip::token_t::tcl_t::END_OF_FILE: {
                setMapInfo(mapValues.data(), rows, cols);
            }
            return true;

            case mip::token_t::tcl_t::BLANK:
            case mip::token_t::tcl_t::COMMENT:
            case mip::token_t::tcl_t::END_OF_LINE:
                break;
            
            case mip::token_t::tcl_t::ATOM:
            case mip::token_t::tcl_t::STRING:
            case mip::token_t::tcl_t::OTHER: {
                switch (st) {
                    case MAP_BEGIN:
                        if (tkn->type() == tcl_t::ATOM && tkn->value() == "{") {
                            st = MAP_VAL;
                            break;
                        }
                        return false;
                    case ANY_KEY:
                        if (tkn->type()==tcl_t::OTHER && tkn->value() == "map") {
                            st = MAP_BEGIN;
                            break;
                        }
                        else if (tkn->type() == tcl_t::OTHER && tkn->value() == "tmap") {
                            st = TEXTURE_BEGIN;
                            break;
                        }
                        return false;
                    case MAP_VAL:
                        if (tkn->type() == tcl_t::ATOM) {
                            if (tkn->value() == ",") {
                                if (cols < 0) {
                                    cols = offset;
                                }
                                else {
                                    if (cols != offset) {
                                        return false;
                                    }
                                }

                                offset = 0;
                                ++rows;
                            }
                            else if (tkn->value() == "}") {
                                st = ANY_KEY;
                                ++rows;
                                break;
                            }
                        }
                        else if (tkn->type() == tcl_t::OTHER) {
                            try {
                                mapValues.push_back(
                                    std::stoll(tkn->value(), 0, 16));
                                ++offset;
                            }
                            catch (...) {
                                return false;
                            }
                        }
                        break;
                    case TEXTURE_BEGIN:
                        if (tkn->type() == tcl_t::ATOM && tkn->value() == "{") {
                            st = TEXTURE_KEY;
                        }
                        break;
                    case TEXTURE_KEY:
                        if (tkn->type() == tcl_t::OTHER) {
                            st = TEXTURE_VALUE;
                            txtKey = tkn->value();
                        }
                        else if (tkn->type() == tcl_t::ATOM && tkn->value() == "}") {
                            st = ANY_KEY;
                        }
                        else {
                            return false;
                        }
                        break;
                    case TEXTURE_VALUE:
                        if (tkn->type() == tcl_t::STRING) {
                            st = TEXTURE_KEY;
                            m_textureList[txtKey] = tkn->value();
                        }
                        else {
                            return false;
                        }
                        break;
                } // switch st
            }
            break;
        }
    } // while

    return false;
}


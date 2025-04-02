/**
 * Copyright (c) 2010 Peter Nitsch
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @author Peter Nitsch
 * http://jsterm.com
 * http://www.peternitsch.net
 *
 * https://github.com/pnitsch/jsTerm
 */

var TERM = TERM || {
	socket : null,
	SERVER_URL : "174.143.154.166", // Your node.js server IP
	SERVER_PORT : "8000", // Your node.js server port
	VERSION : "0.1 alpha"
};

const NULL = 0
const START_OF_HEADING = 1;
const START_OF_TEXT = 2;
const END_OF_TEXT = 3;
const END_OF_TRANSMISSION = 4;
const ENQUIRY = 5;
const ACKNOWLEDGE = 6;
const BELL = 7;
const BACKSPACE = 8;
const HORIZONTAL_TABULATION = 9;
const LINE_FEED = 10;
const VERTICAL_TABULATION = 11;
const FORM_FEED = 12;
const CARRIAGE_RETURN = 13;
const SHIFT_OUT = 14;
const SHIFT_IN = 15;
const DATA_LINK_ESCAPE = 16;
const DEVICE_CONTROL_ONE = 17;
const DEVICE_CONTROL_TWO = 18;
const DEVICE_CONTROL_THREE = 19;
const DEVICE_CONTROL_FOUR = 20;
const NEGATIVE_ACKNOWLEDGE = 21;
const SYNCHRONOUS_IDLE = 22;
const END_OF_TRANSMISSION_BLOCK = 23;
const CANCEL = 24;
const END_OF_MEDIUM = 25;
const SUBSTITUTE = 26;
const ESCAPE = 27;
const FILE_SEPARATOR = 28;
const GROUP_SEPARATOR = 29;
const RECORD_SEPARATOR = 30;
const UNIT_SEPARATOR = 31;
const SPACE = 32;
const EXCLAMATION_MARK = 33;
const QUOTATION_MARK = 34;
const uint_SIGN = 35;
const DOLLAR_SIGN = 36;
const PERCENT_SIGN = 37;
const AMPERSAND = 38;
const APOSTROPHE = 39;
const LEFT_PARENTHESIS = 40;
const RIGHT_PARENTHESIS = 41;
const ASTERISK = 42;
const PLUS_SIGN = 43;
const COMMA = 44;
const HYPHEN_MINUS = 45;
const FULL_STOP = 46;
const SOLIDUS = 47;
const DIGIT_ZERO = 48;
const DIGIT_ONE = 49;
const DIGIT_TWO = 50;
const DIGIT_THREE = 51;
const DIGIT_FOUR = 52;
const DIGIT_FIVE = 53;
const DIGIT_SIX = 54;
const DIGIT_SEVEN = 55;
const DIGIT_EIGHT = 56;
const DIGIT_NINE = 57;
const COLON = 58;
const SEMICOLON = 59;
const LESS_THAN_SIGN = 60;
const EQUALS_SIGN = 61;
const GREATER_THAN_SIGN = 62;
const QUESTION_MARK = 63;
const COMMERCIAL_AT = 64;
const LATIN_CAPITAL_LETTER_A = 65;
const LATIN_CAPITAL_LETTER_B = 66;
const LATIN_CAPITAL_LETTER_C = 67;
const LATIN_CAPITAL_LETTER_D = 68;
const LATIN_CAPITAL_LETTER_E = 69;
const LATIN_CAPITAL_LETTER_F = 70;
const LATIN_CAPITAL_LETTER_G = 71;
const LATIN_CAPITAL_LETTER_H = 72;
const LATIN_CAPITAL_LETTER_I = 73;
const LATIN_CAPITAL_LETTER_J = 74;
const LATIN_CAPITAL_LETTER_K = 75;
const LATIN_CAPITAL_LETTER_L = 76;
const LATIN_CAPITAL_LETTER_M = 77;
const LATIN_CAPITAL_LETTER_N = 78;
const LATIN_CAPITAL_LETTER_O = 79;
const LATIN_CAPITAL_LETTER_P = 80;
const LATIN_CAPITAL_LETTER_Q = 81;
const LATIN_CAPITAL_LETTER_R = 82;
const LATIN_CAPITAL_LETTER_S = 83;
const LATIN_CAPITAL_LETTER_T = 84;
const LATIN_CAPITAL_LETTER_U = 85;
const LATIN_CAPITAL_LETTER_V = 86;
const LATIN_CAPITAL_LETTER_W = 87;
const LATIN_CAPITAL_LETTER_X = 88;
const LATIN_CAPITAL_LETTER_Y = 89;
const LATIN_CAPITAL_LETTER_Z = 90;
const LEFT_SQUARE_BRACKET = 91;
const REVERSE_SOLIDUS = 92;
const RIGHT_SQUARE_BRACKET = 93;
const CIRCUMFLEX_ACCENT = 94;
const LOW_LINE = 95;
const GRAVE_ACCENT = 96;
const LATIN_SMALL_LETTER_A = 97;
const LATIN_SMALL_LETTER_B = 98;
const LATIN_SMALL_LETTER_C = 99;
const LATIN_SMALL_LETTER_D = 100;
const LATIN_SMALL_LETTER_E = 101;
const LATIN_SMALL_LETTER_F = 102;
const LATIN_SMALL_LETTER_G = 103;
const LATIN_SMALL_LETTER_H = 104;
const LATIN_SMALL_LETTER_I = 105;
const LATIN_SMALL_LETTER_J = 106;
const LATIN_SMALL_LETTER_K = 107;
const LATIN_SMALL_LETTER_L = 108;
const LATIN_SMALL_LETTER_M = 109;
const LATIN_SMALL_LETTER_N = 110;
const LATIN_SMALL_LETTER_O = 111;
const LATIN_SMALL_LETTER_P = 112;
const LATIN_SMALL_LETTER_Q = 113;
const LATIN_SMALL_LETTER_R = 114;
const LATIN_SMALL_LETTER_S = 115;
const LATIN_SMALL_LETTER_T = 116;
const LATIN_SMALL_LETTER_U = 117;
const LATIN_SMALL_LETTER_V = 118;
const LATIN_SMALL_LETTER_W = 119;
const LATIN_SMALL_LETTER_X = 120;
const LATIN_SMALL_LETTER_Y = 121;
const LATIN_SMALL_LETTER_Z = 122;
const LEFT_CURLY_BRACKET = 123;
const VERTICAL_LINE = 124;
const RIGHT_CURLY_BRACKET = 125;
const TILDE = 126;
const DELETE = 127;
const LATIN_CAPITAL_LETTER_C_WITH_CEDILLA = 128;
const LATIN_SMALL_LETTER_U_WITH_DIAERESIS = 129;
const LATIN_SMALL_LETTER_E_WITH_ACUTE = 130;
const LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX = 131;
const LATIN_SMALL_LETTER_A_WITH_DIAERESIS = 132;
const LATIN_SMALL_LETTER_A_WITH_GRAVE = 133;
const LATIN_SMALL_LETTER_A_WITH_RING_ABOVE = 134;
const LATIN_SMALL_LETTER_C_WITH_CEDILLA = 135;
const LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX = 136;
const LATIN_SMALL_LETTER_E_WITH_DIAERESIS = 137;
const LATIN_SMALL_LETTER_E_WITH_GRAVE = 138;
const LATIN_SMALL_LETTER_I_WITH_DIAERESIS = 139;
const LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX = 140;
const LATIN_SMALL_LETTER_I_WITH_GRAVE = 141;
const LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS = 142;
const LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE = 143;
const LATIN_CAPITAL_LETTER_E_WITH_ACUTE = 144;
const LATIN_SMALL_LETTER_AE = 145;
const LATIN_CAPITAL_LETTER_AE = 146;
const LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX = 147;
const LATIN_SMALL_LETTER_O_WITH_DIAERESIS = 148;
const LATIN_SMALL_LETTER_O_WITH_GRAVE = 149;
const LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX = 150;
const LATIN_SMALL_LETTER_U_WITH_GRAVE = 151;
const LATIN_SMALL_LETTER_Y_WITH_DIAERESIS = 152;
const LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS = 153;
const LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS = 154;
const CENT_SIGN = 155;
const POUND_SIGN = 156;
const YEN_SIGN = 157;
const PESETA_SIGN = 158;
const LATIN_SMALL_LETTER_F_WITH_HOOK = 159;
const LATIN_SMALL_LETTER_A_WITH_ACUTE = 160;
const LATIN_SMALL_LETTER_I_WITH_ACUTE = 161;
const LATIN_SMALL_LETTER_O_WITH_ACUTE = 162;
const LATIN_SMALL_LETTER_U_WITH_ACUTE = 163;
const LATIN_SMALL_LETTER_N_WITH_TILDE = 164;
const LATIN_CAPITAL_LETTER_N_WITH_TILDE = 165;
const FEMININE_ORDINAL_INDICATOR = 166;
const MASCULINE_ORDINAL_INDICATOR = 167;
const INVERTED_QUESTION_MARK = 168;
const REVERSED_NOT_SIGN = 169;
const NOT_SIGN = 170;
const VULGAR_FRACTION_ONE_HALF = 171;
const VULGAR_FRACTION_ONE_QUARTER = 172;
const INVERTED_EXCLAMATION_MARK = 173;
const LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK = 174;
const RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK = 175;
const LIGHT_SHADE = 176;
const MEDIUM_SHADE = 177;
const DARK_SHADE = 178;
const BOX_DRAWINGS_LIGHT_VERTICAL = 179;
const BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT = 180;
const BOX_DRAWINGS_VERTICAL_SINGLE_AND_LEFT_DOUBLE = 181;
const BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE = 182;
const BOX_DRAWINGS_DOWN_DOUBLE_AND_LEFT_SINGLE = 183;
const BOX_DRAWINGS_DOWN_SINGLE_AND_LEFT_DOUBLE = 184;
const BOX_DRAWINGS_DOUBLE_VERTICAL_AND_LEFT = 185;
const BOX_DRAWINGS_DOUBLE_VERTICAL = 186;
const BOX_DRAWINGS_DOUBLE_DOWN_AND_LEFT = 187;
const BOX_DRAWINGS_DOUBLE_UP_AND_LEFT = 188;
const BOX_DRAWINGS_UP_DOUBLE_AND_LEFT_SINGLE = 189;
const BOX_DRAWINGS_UP_SINGLE_AND_LEFT_DOUBLE = 190;
const BOX_DRAWINGS_LIGHT_DOWN_AND_LEFT = 191;
const BOX_DRAWINGS_LIGHT_UP_AND_RIGHT = 192;
const BOX_DRAWINGS_LIGHT_UP_AND_HORIZONTAL = 193;
const BOX_DRAWINGS_LIGHT_DOWN_AND_HORIZONTAL = 194;
const BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT = 195;
const BOX_DRAWINGS_LIGHT_HORIZONTAL = 196;
const BOX_DRAWINGS_LIGHT_VERTICAL_AND_HORIZONTAL = 197;
const BOX_DRAWINGS_VERTICAL_SINGLE_AND_RIGHT_DOUBLE = 198;
const BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE = 199;
const BOX_DRAWINGS_DOUBLE_UP_AND_RIGHT = 200;
const BOX_DRAWINGS_DOUBLE_DOWN_AND_RIGHT = 201;
const BOX_DRAWINGS_DOUBLE_UP_AND_HORIZONTAL = 202;
const BOX_DRAWINGS_DOUBLE_DOWN_AND_HORIZONTAL = 203;
const BOX_DRAWINGS_DOUBLE_VERTICAL_AND_RIGHT = 204;
const BOX_DRAWINGS_DOUBLE_HORIZONTAL = 205;
const BOX_DRAWINGS_DOUBLE_VERTICAL_AND_HORIZONTAL = 206;
const BOX_DRAWINGS_UP_SINGLE_AND_HORIZONTAL_DOUBLE = 207;
const BOX_DRAWINGS_UP_DOUBLE_AND_HORIZONTAL_SINGLE = 208;
const BOX_DRAWINGS_DOWN_SINGLE_AND_HORIZONTAL_DOUBLE = 209;
const BOX_DRAWINGS_DOWN_DOUBLE_AND_HORIZONTAL_SINGLE = 210;
const BOX_DRAWINGS_UP_DOUBLE_AND_RIGHT_SINGLE = 211;
const BOX_DRAWINGS_UP_SINGLE_AND_RIGHT_DOUBLE = 212;
const BOX_DRAWINGS_DOWN_SINGLE_AND_RIGHT_DOUBLE = 213;
const BOX_DRAWINGS_DOWN_DOUBLE_AND_RIGHT_SINGLE = 214;
const BOX_DRAWINGS_VERTICAL_DOUBLE_AND_HORIZONTAL_SINGLE = 215;
const BOX_DRAWINGS_VERTICAL_SINGLE_AND_HORIZONTAL_DOUBLE = 216;
const BOX_DRAWINGS_LIGHT_UP_AND_LEFT = 217;
const BOX_DRAWINGS_LIGHT_DOWN_AND_RIGHT = 218;
const FULL_BLOCK = 219;
const LOWER_HALF_BLOCK = 220;
const LEFT_HALF_BLOCK = 221;
const RIGHT_HALF_BLOCK = 222;
const UPPER_HALF_BLOCK = 223;
const GREEK_SMALL_LETTER_ALPHA = 224;
const LATIN_SMALL_LETTER_SHARP_S = 225;
const GREEK_CAPITAL_LETTER_GAMMA = 226;
const GREEK_SMALL_LETTER_PI = 227;
const GREEK_CAPITAL_LETTER_SIGMA = 228;
const GREEK_SMALL_LETTER_SIGMA = 229;
const MICRO_SIGN = 230;
const GREEK_SMALL_LETTER_TAU = 231;
const GREEK_CAPITAL_LETTER_PHI = 232;
const GREEK_CAPITAL_LETTER_THETA = 233;
const GREEK_CAPITAL_LETTER_OMEGA = 234;
const GREEK_SMALL_LETTER_DELTA = 235;
const INFINITY = 236;
const GREEK_SMALL_LETTER_PHI = 237;
const GREEK_SMALL_LETTER_EPSILON = 238;
const INTERSECTION = 239;
const IDENTICAL_TO = 240;
const PLUS_MINUS_SIGN = 241;
const GREATER_THAN_OR_EQUAL_TO = 242;
const LESS_THAN_OR_EQUAL_TO = 243;
const TOP_HALF_INTEGRAL = 244;
const BOTTOM_HALF_INTEGRAL = 245;
const DIVISION_SIGN = 246;
const ALMOST_EQUAL_TO = 247;
const DEGREE_SIGN = 248;
const BULLET_OPERATOR = 249;
const MIDDLE_DOT = 250;
const SQUARE_ROOT = 251;
const SUPERSCRIPT_LATIN_SMALL_LETTER_N = 252;
const SUPERSCRIPT_TWO = 253;
const BLACK_SQUARE = 254;
const NO_BREAK_SPACE = 255;

const BLACK_NORMAL = "#000000";
const BLACK_BOLD = "#545454";
const RED_NORMAL = "#a80000";
const RED_BOLD = "#fc5454";
const GREEN_NORMAL = "#00a800";
const GREEN_BOLD = "#54fc54";
const YELLOW_NORMAL = "#a85400";
const YELLOW_BOLD = "#fcfc54";
const BLUE_NORMAL = "#0000a8";
const BLUE_BOLD = "#5454fc";
const MAGENTA_NORMAL = "#a800a8";
const MAGENTA_BOLD = "#fc54fc";
const CYAN_NORMAL = "#00a8a8";
const CYAN_BOLD = "#54fcfc";
const WHITE_NORMAL = "#a8a8a8";
const WHITE_BOLD = "#fcfcfc";

TERM.ByteArray = function (bytes){
    
    this.position = 0;
    this.stringdata = bytes;
    
    this.readUnsignedByte = function(){
        var b = this.stringdata.charCodeAt(this.position);
        if(this.position!=this.stringdata.length) this.position++;
        return b;
    };
    
    this.writeByte = function(){
        // TO DO
    };
    
};

TERM.ByteArray.prototype = {
    
    get bytesAvailable (){
        return this.stringdata.length - this.position;
    },
    get length (){
        return this.stringdata.length;
    }
    
};

TERM.EscapeSequencer = function (viewer){
    
    var viewer = viewer;
    var telnet;
    
    var _customCommand;
    var _currentCustomCommand = {};
    
    this.actionCharacterLib = [];
    
    this.init = function() {
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_H ] = this.cursorPosition;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_F ] = this.cursorPosition;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_A ] = this.cursorUp;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_B ] = this.cursorDown;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_C] = this.cursorForward;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_D ] = this.cursorBackward;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_S ] = this.saveCursorPosition;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_U ] = this.restoreCursorPosition;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_K ] = this.eraseLine;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_J ] = this.eraseDisplay;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_N ] = this.deviceRequest;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_M ] = this.setGraphicsMode;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_H ] = this.setMode;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_L ] = this.resetMode;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_P ] = this.setKeyboardStrings;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_M ] = this.scrollUp;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_R ] = this.scrollScreen;
        
        // TO DO
        this.actionCharacterLib[ LATIN_SMALL_LETTER_A ] = this.unused;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_D ] = this.unused;
        this.actionCharacterLib[ LATIN_SMALL_LETTER_E ] = this.unused;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_L ] = this.unused;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_P ] = this.unused;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_E ] = this.unused;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_F ] = this.unused;
        this.actionCharacterLib[ LATIN_CAPITAL_LETTER_X ] = this.unused;
        
    };
    
    this.executeCommand = function(command) {
        try {
            this.actionCharacterLib[ command[command.length-1] ]( command );
        } catch(error) {
            console.log(error);
        }
    };
    
    this.checkCommandAction = function(position, character) {
        if( this.actionCharacterLib[character] != undefined )
            return true;
        
        return false;
    };
    
    this.unused = function(params) {
        // EMPTY
    };
    
    this.deviceRequest = function(params) {
        if( params[2]==DIGIT_FIVE ){
            TERM.socket.writeByte(ESCAPE);
            TERM.socket.writeByte(LEFT_SQUARE_BRACKET);
            TERM.socket.writeByte(DIGIT_ZERO);
            TERM.socket.writeByte(LATIN_SMALL_LETTER_N);
        } else if( params[2]==DIGIT_SIX ) {
            var i;
            var rows = "" + viewer.cursor.y;
            var cols = "" + viewer.cursor.x;
            
            TERM.socket.writeByte(ESCAPE);
            TERM.socket.writeByte(LEFT_SQUARE_BRACKET);
            for(i=0;i<rows.length;i++)    TERM.socket.writeByte(rows.charCodeAt(i));
            TERM.socket.writeByte(SEMICOLON);
            for(i=0;i<cols.length;i++)    TERM.socket.writeByte(cols.charCodeAt(i));
            TERM.socket.writeByte(LATIN_CAPITAL_LETTER_R);
        } else {
            // 0 - Report Device OK
            // 3 - Report Device Failure
        }
    };
    
    this.cursorPosition = function(params) {
        var lastCharacter = params[params.length-1];
        
        if( params.length==3 && lastCharacter==LATIN_CAPITAL_LETTER_H){
            viewer.home();
        } else {
            var lineArray = [];
            var lineStr = "";
            var line = 0;
            
            var columnArray = [];
            var columnStr = "";
            var column = 0;
            
            if(params.indexOf(SEMICOLON) != -1){
                var semicolonIndex = params.indexOf(SEMICOLON);
                
                if( params[semicolonIndex-1] != LEFT_SQUARE_BRACKET ) {
                    lineArray = params.slice(2, semicolonIndex);
                    for( i=0; i<lineArray.length; i++ ){
                        lineStr += (lineArray[i] - 48).toString();
                    }
                    line = parseInt(lineStr);
                }
                
                columnArray = params.slice(semicolonIndex+1, params.length-1);
                for( i=0; i<columnArray.length; i++ ){
                    columnStr += (columnArray[i] - 48).toString();
                }
                column = parseInt(columnStr);
                
            } else if(params.slice(2, params.indexOf(lastCharacter)).length > 0){
                lineArray = params.slice(2, params.length-1);
                for( i=0; i<lineArray.length; i++ ){
                    lineStr += (lineArray[i] - 48).toString();
                }
                line = parseInt(lineStr);
            }
            
            column = (column>0) ? column-1 : 0;
            line = (line>0) ? line-1 : 0;
            
            viewer.reposition(column, line);
        }
    };
    
    this.cursorUp = function(params) {
        var valueArray = params.slice(2, params.length-1);
        var valueStr = "";
        for( i=0; i<valueArray.length; i++ ){
            valueStr += (valueArray[i] - 48).toString();
        }
        var value = (valueStr.length > 0) ? parseInt(valueStr) : 1;
        
        viewer.moveUp(value);
    };
    
    this.cursorDown = function(params) {
        var valueArray = params.slice(2, params.length-1);
        var valueStr = "";
        for( i=0; i<valueArray.length; i++ ){
            valueStr += (valueArray[i] - 48).toString();
        }
        var value = (valueStr.length > 0) ? parseInt(valueStr) : 1;
        
        viewer.moveDown(value);
    };
    
    this.cursorForward = function(params) {
        var valueArray = params.slice(2, params.length-1);
        var valueStr = "";
        for( i=0; i<valueArray.length; i++ ){
            valueStr += (valueArray[i] - 48).toString();
        }
        var value = (valueStr.length > 0) ? parseInt(valueStr) : 1;
        
        viewer.moveForward(value);
    };
    
    this.cursorBackward = function(params) {
        var valueArray = params.slice(2, params.length-1);
        var valueStr = "";
        for( i=0; i<valueArray.length; i++ ){
            valueStr += (valueArray[i] - 48).toString();
        }
        var value = (valueStr.length > 0) ? parseInt(valueStr) : 1;
        
        viewer.moveBackward(value);
    };
    
    this.saveCursorPosition = function(params) {
        viewer.savePosition();
    };
    
    this.restoreCursorPosition = function(params) {
        viewer.restorePosition();
    };
    
    // Set Graphic Mode functions
    var _bold = false;
    var _reverse = false;
    
    var _boldColors = [BLACK_BOLD, RED_BOLD, GREEN_BOLD, YELLOW_BOLD, BLUE_BOLD, MAGENTA_BOLD, CYAN_BOLD, WHITE_BOLD];
    var _normalColors = [BLACK_NORMAL, RED_NORMAL, GREEN_NORMAL, YELLOW_NORMAL, BLUE_NORMAL, MAGENTA_NORMAL, CYAN_NORMAL, WHITE_NORMAL];
    
    var _currentForegroundColor = WHITE_NORMAL;
    var _currentBackgroundColor = BLACK_NORMAL;
    
    this.setGraphicsMode = function(params) {
        for( i=2; i<params.length; i++ ){
            switch( params[i] ){
                    
                    /*  Reset */
                case LATIN_SMALL_LETTER_M:
                case DIGIT_ZERO:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET){
                        _bold = false;
                        _reverse = false;
                        
                        _currentForegroundColor = WHITE_NORMAL;
                        _currentBackgroundColor = BLACK_NORMAL;
                        
                        viewer.foregroundColorChanged(_currentForegroundColor);
                        viewer.backgroundColorChanged(_currentBackgroundColor);
                    }
                    break;
                    
                    /*  Bold ON */
                case DIGIT_ONE:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET) {
                        _bold = true;
                        
                        for( j=0; j<_normalColors.length; j++ ){
                            if( _currentForegroundColor == _normalColors[j] )
                                _currentForegroundColor = _boldColors[j];
                        }
                        
                        viewer.foregroundColorChanged(_currentForegroundColor);
                    }
                    break;
                    
                    /* Dim */
                case DIGIT_TWO:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET) {
                        _bold = false;
                        
                        for( j=0; j<_normalColors.length; j++ ){
                            if( _currentForegroundColor == _boldColors[j] )
                                _currentForegroundColor = _normalColors[j];
                        }
                        
                        viewer.foregroundColorChanged(_currentForegroundColor);
                    }
                    break;
                    
                    /* Set foreground color */
                case DIGIT_THREE:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET){
                        if(params[i+1] != SEMICOLON && params[i+1] != LATIN_SMALL_LETTER_M){
                            
                            var position = params[i+1] - 48;
                            if(_reverse) {
                                _currentBackgroundColor = _normalColors[position];
                                viewer.backgroundColorChanged(_currentBackgroundColor);
                            }else {
                                _currentForegroundColor = (_bold) ? _boldColors[position] : _normalColors[position];
                                viewer.foregroundColorChanged(_currentForegroundColor);
                            }
                            
                        }
                    }
                    break;
                    
                    /* Set background color */
                case DIGIT_FOUR:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET){
                        if(params[i+1] != SEMICOLON && params[i+1] != LATIN_SMALL_LETTER_M){
                            position = params[i+1] - 48;
                            if(_reverse) {
                                _currentForegroundColor = (_bold) ? _boldColors[position] : _normalColors[position];
                                viewer.foregroundColorChanged(_currentForegroundColor);
                            } else {
                                _currentBackgroundColor = _normalColors[position];
                                viewer.backgroundColorChanged(_currentBackgroundColor);
                            }
                            
                            /* Underline ON */
                        } else {
                            // TO DO
                        }
                    }
                    break;
                    
                    /* Blink ON */
                case DIGIT_FIVE:
                    // TO DO
                    break;
                    
                    /* Reverse ON */
                case DIGIT_SEVEN:
                    if(params[i-1] == SEMICOLON || params[i-1] == LEFT_SQUARE_BRACKET)
                        _reverse = true;
                    break;
                    
                    /* Concealed ON */
                case DIGIT_EIGHT:
                    // TO DO
                    break;
                    
                    /* Reset to normal? */
                case DIGIT_NINE:
                    // TO DO
                    break;
            }
        }
    };
    
    this.scrollScreen = function(params) {
        if(params.length==3){
            viewer.scrollScreen();
        } else {
            var lastCharacter = params[params.length-1];
            
            var sArray = [];
            var sStr = "";
            var s = 0;
            
            var eArray = [];
            var eStr = "";
            var e = 0;
            
            var semicolonIndex = params.indexOf(SEMICOLON);
            
            sArray = params.slice(2, semicolonIndex);
            for( i=0; i<sArray.length; i++ ){
                sStr += (sArray[i] - 48).toString();
            }
            s = parseInt(sStr);
            
            
            eArray = params.slice(semicolonIndex+1, params.length-1);
            for( i=0; i<eArray.length; i++ ){
                eStr += (eArray[i] - 48).toString();
            }
            e = parseInt(eStr);
            
            viewer.scrollScreen(s, e);
        }
    };
    
    this.scrollUp = function(params) {
        viewer.scrollUp(1);
    };
    
    this.eraseLine = function(params) {
        if( params[2]==DIGIT_ONE ){
            viewer.eraseStartOfLine();
        } else if( params[2]==DIGIT_TWO ) {
            viewer.eraseLine();
        } else {
            viewer.eraseEndOfLine();
        }
    };
    
    this.eraseDisplay = function(params) {
        if( params[2]==DIGIT_ONE ){
            viewer.eraseUp();
            viewer.reposition(0, 0);
        } else if( params[2]==DIGIT_TWO ) {
            viewer.eraseScreen();
            viewer.reposition(0, 0);
        } else {
            viewer.eraseDown();
        }
    };
    
    // Terminal functions
    this.setMode = function(){
        // TO DO
    };
    
    this.resetMode = function(){
        // TO DO
    };
    
    this.setKeyboardStrings = function(){
        // TO DO
    };
    
    this.init();
    
};

TERM.AnsiParser = function (viewer){
    
    var viewer = viewer;
    var escapeCommand = [];
    var bufferEscapeCommand = false;
    
    this.escapeCommands = new TERM.EscapeSequencer(viewer);
    this._bytes;
    
    this.parse = function (bytes) {
        
        if( bytes != null ) this._bytes = new TERM.ByteArray(bytes);
        
        while( this._bytes.bytesAvailable > 0 ){
            var result = this._bytes.readUnsignedByte();
            
            if( result == SUBSTITUTE) {
                break;
            } else {
                this.readByte( result );
            }
        }
        
        this._bytes.position = 0;
    };
    
    this._exceptionsLib = [];
    this._exceptions = [];
    
    this.hasException = function( code ) {
        if( this._exceptions.indexOf(code) != -1 )
            return true;
        return false;
    };
    
    this.writeException = function( code, callback ) {
        if( !this.hasException(code) ){
            this._exceptionsLib[code] = callback;
            this._exceptions.push(code);
        }
    };
    
    this.readByte = function (b) {
        
        if(b == ESCAPE) {
            escapeCommand = [];
            escapeCommand.push(b);
            bufferEscapeCommand = true;
        } else {
            if(bufferEscapeCommand){
                escapeCommand.push(b);
                if( this.escapeCommands.checkCommandAction(escapeCommand.length, b) ) {
                    this.escapeCommands.executeCommand(escapeCommand);
                    bufferEscapeCommand = false;
                }
            } else if( this.hasException(b) ) {
                this._exceptionsLib[b]( b, this._bytes );
            } else if(b >= SPACE) {
                viewer.drawCharacter(b);
            } else {
                switch(b) {
                    case BACKSPACE:
                        viewer.moveBackward(1, true);
                        break;
                        
                    case LINE_FEED:
                        viewer.carriageReturn();
                        viewer.moveDown(1);
                        break;
                        
                    case CARRIAGE_RETURN:
                        viewer.carriageReturn();
                        break;
                        
                    case FORM_FEED:
                        viewer.eraseScreen();
                        viewer.reposition(0, 0);
                        break;
                }
            }
        }
    };
    
};

TERM.Point = function() {
    
    this.x = 0;
    this.y = 0;
    
};

TERM.Font = function() {
    
    this.width = 8;
    this.height = 16;
    this.lineHeight = 23;
    
};

TERM.Cursor = function (){
    
    this.foregroundColor = WHITE_NORMAL;
    this.backgroundColor = BLACK_NORMAL;
    this.position = new TERM.Point();
    this.maxColumnWidth = 80;
    this.maxLineHeight = 25;
    this.columnWidth = 8;
    this.lineHeight = 16;
    this.maxColumns = 80;
    this.infiniteWidth = false;
    this.infiniteHeight = false;
    
    this.moveForward = function(columns) {
        if( this.position.x + (columns*this.columnWidth) <= this.maxColumns * this.columnWidth )
            this.position.x = this.position.x + (columns*this.columnWidth);
        else
            this.position.x = (this.maxColumns * this.columnWidth) - this.columnWidth;
    };
    
    this.moveBackward = function(columns) {
        if( this.position.x - (columns*this.columnWidth) >= 0 )
            this.position.x = this.position.x - (columns*this.columnWidth);
        else
            this.position.x = 0;
    };
    
    this.moveDown = function(lines) {
        this.position.y = this.position.y + (lines*this.lineHeight);
    };
    
    this.moveUp = function(lines) {
        if( this.position.y - (lines*this.lineHeight) >= 0 )
            this.position.y = this.position.y - (lines*this.lineHeight);
        else
            this.position.y = 0;
    };
    
    this.carriageReturn = function() {
        this.position.x = 0;
    };
    
};

TERM.Cursor.prototype = {
    
    get x (){
        return this.position.x;
    },
    set x (val){
        this.position.x = val;
    },
    
    get y (){
        return this.position.y;
    },
    set y (val){
        this.position.y = val;
    }
    
};

/**
 * @author Peter Nitsch
 */

TERM.AnsiViewer = function (fontmap){
    
    this.cursor = new TERM.Cursor();
    this.parser = new TERM.AnsiParser(this);
    
    this.cursor_visible = 0;
    this.cursor_enabled = 1;
    
    this.savedData = null;
    
    var fontmap = fontmap;
    var font = new TERM.Font();
    var canvas = document.getElementById("canvas");
    var width = canvas.width;
    var height = canvas.height;
    var topMargin = 1;
    var botMargin = 25;
    var ctx = canvas.getContext("2d");
    var scroll = true;
    var _savedPosition = new TERM.Point();
    
    this.readBytes = function (bytes) {
        this.parser.parse(bytes);
    };
    
    this.clearCanvas = function(){
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, 0, width, height);
    };
    
    this.colorTable = function(val) {
        switch(val) {
            case BLACK_NORMAL: return 0; break;
            case BLUE_NORMAL: return 1; break;
            case GREEN_NORMAL: return 2; break;
            case CYAN_NORMAL: return 3; break;
            case RED_NORMAL: return 4; break;
            case MAGENTA_NORMAL: return 5; break;
            case YELLOW_NORMAL: return 6; break;
            case WHITE_NORMAL: return 7; break;
            case BLACK_BOLD: return 8; break;
            case BLUE_BOLD: return 9; break;
            case GREEN_BOLD: return 10; break;
            case CYAN_BOLD: return 11; break;
            case RED_BOLD: return 12; break;
            case MAGENTA_BOLD: return 13; break;
            case YELLOW_BOLD: return 14; break;
            case WHITE_BOLD: return 15; break;
        }
        return 0;
    };
    
    this.drawCharacter = function(character) {
        this.removeCursor();
        this.draw(character);
        this.cursor.moveForward(1);
        
        if(!this.cursor.infinitewidth && this.cursor.x + this.cursor.columnWidth > this.cursor.maxcolumnWidth * this.cursor.columnWidth){
            this.moveDown(1);
            this.cursor.carriageReturn();
        }
        this.drawCursor();
    };
    
    this.draw = function(charCode) {
        //        console.log(charCode +" "+ this.cursor.x +" "+ this.cursor.y)
        
        ctx.fillStyle = this.cursor.backgroundColor;
        ctx.fillRect(this.cursor.x, this.cursor.y, font.width, font.height);
        
        ctx.drawImage(fontmap,
                      charCode*(font.width+1), this.colorTable(this.cursor.foregroundColor)*font.height, font.width, font.height,
                      this.cursor.x, this.cursor.y, font.width, font.height);
    };
    
    this.carriageReturn = function() {
        this.removeCursor();
        this.cursor.carriageReturn();
        this.drawCursor();
    };
    
    this.formFeed = function() {
        this.removeCursor();
        this.cursor.x = 0;
        this.cursor.y = 0;
        this.drawCursor();
    };
    
    this.moveBackward = function(val, erase) {
        var movements = val;
        
        this.removeCursor();
        while( movements > 0 ) {
            this.cursor.moveBackward(1);
            if(erase) this.draw(SPACE);
            movements--;
        }
        this.drawCursor();
    };
    
    this.moveDown = function(val) {
        this.removeCursor();
        if(this.cursor.y >= this.cursor.lineHeight*(botMargin-1) && scroll){
            this.scrollUp(1);
        } else {
            this.cursor.moveDown(val);
        }
        this.drawCursor();
    };
    
    this.moveForward = function(val) {
        this.removeCursor();
        this.cursor.moveForward(val);
        this.drawCursor();
    };
    
    this.moveUp = function(val) {
        this.removeCursor();
        this.cursor.moveUp(val);
        this.drawCursor();
    };
    
    this.reposition = function(x, y) {
        this.removeCursor();
        this.cursor.x = x * this.cursor.columnWidth;
        this.cursor.y = y * this.cursor.lineHeight;
        this.drawCursor();
    };
    
    this.restorePosition = function() {
        this.removeCursor();
        this.cursor.x = _savedPosition.x;
        this.cursor.y = _savedPosition.y;
        this.drawCursor();
    };
    
    this.savePosition = function() {
        _savedPosition.x = this.cursor.x;
        _savedPosition.y = this.cursor.y;
    };
    
    this.displayCleared = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, 0, this.cursor.maxcolumnWidth * this.cursor.columnWidth, this.cursor.maxlineHeight * this.cursor.lineHeight);
        this.drawCursor();
    };
    
    this.eraseUp = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, 0, this.cursor.maxcolumnWidth * this.cursor.columnWidth, this.cursor.y);
        this.drawCursor();
    };
    
    this.eraseScreen = function() {
        this.removeCursor();
        ctx.fillStyle = this.cursor.backgroundColor;
        ctx.fillRect(0, 0, this.cursor.maxcolumnWidth * this.cursor.columnWidth, this.cursor.maxlineHeight * this.cursor.lineHeight);
        this.drawCursor();
    };
    
    this.eraseDown = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, this.cursor.y, this.cursor.maxcolumnWidth * this.cursor.columnWidth, (this.cursor.maxlineHeight * this.cursor.lineHeight) - this.cursor.y);
        this.drawCursor();
    };
    
    this.eraseEndOfLine = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        var w = (this.cursor.maxcolumnWidth * this.cursor.columnWidth) - (this.cursor.x - this.cursor.columnWidth);
        ctx.fillRect(this.cursor.x, this.cursor.y, w, this.cursor.lineHeight);
        this.drawCursor();
    };
    
    this.eraseStartOfLine = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, this.cursor.y, this.cursor.x, this.cursor.lineHeight);
        this.drawCursor();
    };
    
    this.eraseLine = function() {
        this.removeCursor();
        ctx.fillStyle = BLACK_NORMAL;
        ctx.fillRect(0, this.cursor.y, this.cursor.maxcolumnWidth * this.cursor.columnWidth, this.cursor.lineHeight);
        this.drawCursor();
    };
    
    this.backgroundColorChanged = function(color) {
        this.cursor.backgroundColor = color;
    };
    
    this.foregroundColorChanged = function(color) {
        this.cursor.foregroundColor = color;
    };
    
    this.home = function() {
        this.removeCursor();
        this.cursor.x = 0;
        this.cursor.y = (topMargin-1) * this.cursor.maxlineHeight;
        this.drawCursor();
    };
    
    this.scrollScreen = function(start, end) {
        this.removeCursor();
        topMargin = start;
        botMargin = end;
        
        handleHome();
        this.drawCursor();
    };
    
    this.scrollUp = function(val) {
        this.removeCursor();
        var canvasData = ctx.getImageData(0, topMargin * this.cursor.lineHeight, this.cursor.maxcolumnWidth*this.cursor.columnWidth, this.cursor.lineHeight * (botMargin-topMargin));
        this.displayCleared();
        ctx.putImageData(canvasData, 0, this.cursor.lineHeight*(topMargin-1));
        this.drawCursor();
    };
    
    this.drawCursor = function() {
        if (this.cursor_enabled && this.cursor_visible == 0) {
/*            cursorSavedData = ctx.getImageData(this.cursor.x, this.cursor.y, this.cursor.columnWidth, this.cursor.lineHeight);*/
            ctx.fillStyle = "#ffdfbf";
            ctx.fillRect(this.cursor.x, this.cursor.y + font.height - 2, font.width, 2);
            this.cursor_visible = 1;
        }
    }

    this.removeCursor = function() {
        if (this.cursor_enabled && this.cursor_visible == 1) {
/*            ctx.putImageData(cursorSavedData, this.cursor.x, this.cursor.y);*/
            ctx.fillStyle = "#000000";
            ctx.fillRect(this.cursor.x, this.cursor.y + font.height - 2, font.width, 2);
            this.cursor_visible = 0;
        }
    }
    
    this.clearCanvas();
    
};

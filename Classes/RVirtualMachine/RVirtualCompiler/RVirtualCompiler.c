#include "RVirtualCompiler.h"
#include "../RVirtualMachine/RVirtualMachine.h"

constructor(RVirtualCompiler)) {
    object = allocator(RVirtualCompiler);
    if(object != nil) {

        object->classId = registerClassOnce(toString(RVirtualCompiler));
        object->lines = 1;
        object->symbols = 1;
        object->numberOfLines = 0;
    }
    return object;
}

destructor(RVirtualCompiler) {
}

singleton(RVirtualCompiler) {
    static RVirtualCompiler *instance = nil;
    if(instance == nil) {
        instance = $(nil, c(RVirtualCompiler)));
    }
    return instance;
}

method(RCString *, getFunctionName, RVirtualCompiler)) {
    if(object->code->size != 0) {
        size_t place;

        // finding to startSymbol
        place = indexOfFirstCharacterCString(object->code->baseString, object->code->size, ':');

        // if there is ':'
        if(place != object->code->size) {

            // get copy of substring
            RCString *name = $(object->code, m(substringInRange, RCString)), makeRRange(0, place));

            // delete nameString start sourceCode and ':' symbol
            $(object->code, m(deleteInRange, RCString)), makeRRange(0, place + 1));
            return name;
        }
    }
    return nil;
}

#pragma mark Brainfuck lang to rasm

method(byte, brainFuckSourceToByteCode, RVirtualCompiler)) {
    byte byteCode;

    switch (object->code->baseString[object->iterator]) {

        case '+': {
            byteCode = r_increment;
        } break;

        case '-': {
            byteCode = r_decrement;
        } break;

        case '.': {
            byteCode = r_print_char;
        } break;

        case '>': {
            byteCode = r_move_forward;
        } break;

        case '<': {
            byteCode = r_move_backward;
        } break;

        case ',': {
            byteCode = r_get_char;
        } break;

        case '[': {
            // complicated case
            size_t realPath;

            object->deltaToNext = indexOfLastCharacterCString(&object->code->baseString[object->iterator + object->iteratorShift], object->code->size - object->deltaToNext, ']');
            --object->forwardRepetitions;
            realPath = object->iterator + object->iteratorShift + object->deltaToNext + (object->forwardRepetitions + object->backwardRepetitions) * 2;

            if(realPath > 255) {
                RPrintf("%p Warning. RVirtualCompiler (BrainFuck). '[' Too long loop", object);
            }

            object->body->array[object->iterator + object->iteratorShift]     = r_if;
            object->body->array[object->iterator + object->iteratorShift + 1] = r_goto_address;
            object->body->array[object->iterator + object->iteratorShift + 2] = (byte) realPath;

            object->iteratorShift += 2;
            byteCode = r_ignore;
        } break;

        case ']': {
            // complicated case
            size_t realPath;

            object->toPrev = indexOfLastCharacterCString(object->code->baseString, object->toPrev ? object->toPrev : object->code->size, '[');
            --object->backwardRepetitions;
            realPath = object->toPrev + (object->forwardRepetitions + object->backwardRepetitions) * 2;

            if(realPath > 255) {
                RPrintf("%p Warning. RVirtualCompiler (BrainFuck). ']' Too long loop", object);
            }

            object->body->array[object->iterator + object->iteratorShift]     = r_if_not;
            object->body->array[object->iterator + object->iteratorShift + 1] = r_goto_address;
            object->body->array[object->iterator + object->iteratorShift + 2] = (byte) realPath;

            object->iteratorShift += 2;
            byteCode = r_ignore;
        } break;

        case '\n': {
            ++object->lines;
            object->symbols = 1;
            --object->iteratorShift;
            byteCode = r_ignore;
        } break;

        case ' ': {
            --object->iteratorShift;
            byteCode = r_ignore;
        } break;

        default: {
            byteCode = r_end;
            RPrintf("Warning. RVirtualCompiler (BrainFuck). Undefined symbol on line: %lu, place: %lu !\n",
                                                                            object->lines, object->symbols);
        }
    }

    ++object->symbols;
    return byteCode;
}

method(RByteArray *, getBrainFuckFunctionBody, RVirtualCompiler)) {
    RByteArray *body;
    byte        character;
    size_t      sizeOfByteCode;
    object->iterator      = 0;
    object->iteratorShift = 0; // shift cause '[' and ']' 3x multiplience

    // delete spaces
    $(object->code, m(deleteAllCharacters, RCString)), ' ');
    object->numberOfLines = $(object->code, m(numberOfCharacters, RCString)), '\n');

    // all [, ] instructions will be doubled in the byte-code,
    // because of r_if instruction build
    object->forwardRepetitions  = $(object->code, m(numberOfCharacters, RCString)), '[');
    object->backwardRepetitions = $(object->code, m(numberOfCharacters, RCString)), ']');

    if(object->forwardRepetitions != object->backwardRepetitions) {
        RPrintf("Warning. RVirtualCompiler (BrainFuck). Count of \'[\' and \']\' isn't equal!");
    }

    // removing all '\n' start byte-code, +1 to r_end
    sizeOfByteCode = object->code->size - object->numberOfLines + 1 + 2 * (object->forwardRepetitions + object->backwardRepetitions);

    body = makeRByteArray(sizeOfByteCode);

    // set pointer to body
    object->body = body;

    while(object->iterator < object->code->size) {
        character = $(object, m(brainFuckSourceToByteCode, RVirtualCompiler)));

        if(character != r_ignore) {
            body->array[object->iterator + object->iteratorShift] = character;
        }

        ++object->iterator;
    }

    body->array[object->iterator + object->iteratorShift] = r_end;

    return body;
}

method(RVirtualFunction *, createFunctionFromBrainFuckSourceCode, RVirtualCompiler), const RCString *sourceCode) {

    if(sourceCode->size != 0) {
        RVirtualFunction *function = $(nil, c(RVirtualFunction)) );

        // copy source to object
        object->code = $(sourceCode, m(copy, RCString)) );

        // init workings
        object->deltaToNext = 0;
        object->toPrev      = 0;

        // set name and body
        function->name               = $(object, m(getFunctionName,          RVirtualCompiler)) );
        master(function, RByteArray) = $(object, m(getBrainFuckFunctionBody, RVirtualCompiler)) );

        if(function->name == nil) {
            function->name = RSC("Unnamed");
        }
        deleter(object->code, RCString);

        RPrintf("RVirtualCompiler. Brainfuck. Processed lines - %lu of %lu, in %lu iterations \n", object->lines, object->numberOfLines + 1, object->iterator);
        // print result for debug
        //  $(function, p(RVirtualFunction)) );
        return function;
    } else {
        RPrintf("Error. RVirtualCompiler. Bad virtual-code size\n");
    }
    return nil;
}
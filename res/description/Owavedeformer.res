CONTAINER Owavedeformer {
    NAME Owavedeformer;
    
    INCLUDE Obase;
    GROUP ID_OBJECTPROPERTIES {
        COLUMNS 1;
        REAL WAVE_TWIST_AMOUNT { UNIT REAL; MIN 0.0; STEP 0.1; }
        REAL WAVE_WIDTH { UNIT	METER; MIN 0.0; STEP 1.0; }
        REAL WAVE_LENGTH { UNIT	METER; MIN 0.0; STEP 1.0; }
        GRADIENT WAVE_FALLOFF { COLOR; }
        SHADERLINK WAVE_TEXTURE {  }
        VECTOR WAVE_TEXTURE_OFFSET { UNIT REAL; }
        REAL WAVE_BEND { UNIT REAL; MINSLIDER 0.0; MAXSLIDER 1.0; CUSTOMGUI REALSLIDER; STEP 0.01; }
    }
    GROUP ID_ADVANCED {
        COLUMNS 1;
        BOOL WAVE_USE_LENGTH_FALLOFF {  }
        GRADIENT WAVE_LENGTH_FALLOFF {  }
        BOOL WAVE_USE_MASK {  }
        SHADERLINK WAVE_MASK {  }
    }
    GROUP ID_PREVIEW {
        COLUMNS 1;
        BOOL WAVE_PREVIEW {  }
        BOOL WAVE_SIZE_PREVIEW {  }
    }
}
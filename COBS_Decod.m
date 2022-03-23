function Decod_Data = COBS_Decod(Input_Data)

    %%  Note: Input_Data    -   Data in format uint8
    %         Decod_Data    -   Data to code COBS uint8
    
    %%  Def group
    
    Input               =   1;
    Output              =   2;
    
    %%  Define Position Point
    
    Previous            =   @(Position)         Position + -1;
    Offset              =   @(Position, Offset) Position + Offset;

    %%  Initial parameter
    
    Length{Input}       =   length(Input_Data);
    Decod_Data          =   zeros(1, 2 ^ floor(log2(Length{Input})),...
                            'uint8');
                             
    %%  Program Realization
    
%     Pointer{Input}      =   1;
    Pointer{Output}     =   1;
    
    while(~isempty( Input_Data(Input_Data ~= 0)) )
        
        Zero_Point      =   int32(Input_Data(1));
        
        Input_Data(1)   =   [];
        
        Decod_Data(Pointer{Output}: Offset(Pointer{Output},        ...
                                           Offset(Zero_Point, -2)))...
                        =   Input_Data(1: Previous(Zero_Point));
        
        if (Zero_Point  ~= 255)
            
            Pointer{Output}                                        ...
                        =   Offset(Pointer{Output},                ...
                                   Zero_Point);
        
        else
            
            Pointer{Output}                                        ...
                        =   Offset(Pointer{Output},                ...
                                   Previous(Zero_Point));
                               
        end
        
        Input_Data(1: Previous(Zero_Point))                        ...
                        =   [];

    end
    
    Decod_Data(Previous(Pointer{Output}): end)                     ...
                        =   [];
                    
end
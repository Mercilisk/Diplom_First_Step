function Encod_Data = COBS_Encode(Input_Data)

    %%  Note: Input_Data    -   Data in format uint8
    %         Encod_Data    -   Data to code COBS uint8
    
    %%  Def group
    
    Input               =   1;
    Output              =   2;
    
    %%  Define Position Point
    
    Previous            =   @(Position)         Position + -1;
    Next                =   @(Position)         Position + 1;
    Offset              =   @(Position, Offset) Position + Offset;

    %%  Initial parameter
    
    Length{Input}       =   length(Input_Data);
    Encod_Data          =   zeros(1, Length{Input} + 1 +           ...
                                 ceil(Length{Input} / 256), 'uint8');
                             
    %%  Program Realization
    
    Pointer{Output}     =   1;
    
    while(1)
        
        Zero_Point      =   min([255,find([Input_Data, 0] == 0)]);
        
        Encod_Data(Pointer{Output})                                ...
                        =   Zero_Point;
        
        Encod_Data(Next(Pointer{Output}): Offset(Pointer{Output},  ...
                                           Previous(Zero_Point)))  ...
                        =   Input_Data(1: Previous(Zero_Point));
        if (Zero_Point > length(Input_Data))
            
            break;
            
        end
        Input_Data(1)   =   [];
        if (Zero_Point ~= 255)
            
            Input_Data(1: Offset(Zero_Point, -1))                  ...
                        =   [];
        else
            Input_Data(1: Offset(Zero_Point, -2))                  ...
                        =   [];
        end

%         if (Input_Data(1) == 0 && Zero_Point < 255)
%             
%             Input_Data(1)                                          ...
%                         =   [];
%                     
%         end
        Pointer{Output} =   Offset(Pointer{Output}, Zero_Point); 
        
    end
    
end
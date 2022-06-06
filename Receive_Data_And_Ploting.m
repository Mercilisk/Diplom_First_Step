classdef Receive_Data_And_Ploting < matlab.apps.AppBase

    %% Properties that correspond to app components
    properties (Access = public)
        Figure_Handler      	matlab.ui.Figure
        Axes                   	matlab.ui.control.UIAxes
        ConfigurationPanel      matlab.ui.container.Panel
        SerialPortDropDown      matlab.ui.control.DropDown
        SerialPortDropDownLabel matlab.ui.control.Label
        MeasurementButton       matlab.ui.control.Button
        BitRateKnobLabel        matlab.ui.control.Label
        BitRateKnob             matlab.ui.control.DiscreteKnob
        CellGetNumber           matlab.ui.control.EditField
    end
    
    %% Ðroperties that correspond to constant and variable components
    properties (Access = private)
        %%  Define
    
        Color_Background    =   [0, 255, 255] ./ 255;
        
        Activate            =   1;
        DeActivate          =   0;
        
        %%    Constant and Variable
        
        Port                                % Port Information
        
        Length_Signal       =   512         % Length plot Discrete
        Line_Handle                         % Handle Axes object
        Frame_Transmit                      % Transmit good data
        Length_Input_Buffer
        
        Input_Data          =   zeros(1,                           ...
                                      512); % Data receive RS232
        Signal                              % Data decod RS232
        Y_Data              =   [];         % Data decod to ploting
       
    end
    
    %% Programm methods that correspond to Update Propertise
    methods (Access = private)
        
        %%  Update Axis property
        function Data_Collector_And_Plottin(app)
            
            cla(app.Axes);

            set(app.Line_Handle, 'YData', app.Y_Data);
            
        end
        
        %%  Update Ploting data
        function Y_Data = Transform_Read_Data(app, Y_Data, Signal)

            if (isempty(Y_Data))

                Y_Data      =   zeros(1, app.Length_Signal);

                Y_Data(1: length(Signal))                          ...
                          	=   Signal;

                app.Y_Data_Real_Length                             ...
                          	=   size(Signal);

            elseif (app.Y_Data_Real_Length >= app.Length_Signal)

                Y_Data(1: end - length(Signal))                	   ...
                          	=   Y_Data(length(Signal): end);

                Y_Data(length(Signal): end)                        ...
                          	=   Signal;

                app.Y_Data_Real_Length                             ...
                          	=   app.Y_Data_Real_Length -           ...
                                    length(Signal);

            else

                Y_Data(1: length(Signal))                          ...
                          	=   Signal;

                app.Y_Data_Real_Length                             ...
                         	=   app.Y_Data_Real_Length +           ...
                                	length(Signal);

            end

        end
  
        %%  Update Receive RS232
        
        function Read_Data(app)
            
            app.Input_Data                                         ...
                      	=   fread(app.Port,app.Length_Input_Buffer,...
                            'uint8');
                    
            Zero_Pos    =   find(app.Input_Data == 0);
            
            Zero_Pos(2:end + 1)                                    ...
                        =   Zero_Pos;
            Zero_Pos(1) =   0;
            
            for Number_Transaction = 2: length(Zero_Pos)
                
                Transaction                                        ...
                        =   app.Input_Data(                        ...
                                Zero_Pos(Number_Transaction - 1)+1:...
                                Zero_Pos(Number_Transaction));
                            
                
                File_ID     =   fopen(['Data\Data',                ...
                                       num2str(Number_Transaction),...
                                       '.txt'],'a');
                fprintf(File_ID,'%d\n',Transaction');
                fclose(File_ID);
                
            end
            
        end
        
%         function Read_Data(app)
%             
%             if (app.Frame_Transmit == app.DeActivate)
%                 
%                 app.Input_Data(1)                                  ...
%                             =   fread(app.Port, 1, 'uint8');
%                         
%                 if (app.Input_Data(1) == 0)
%                     
%                     app.Frame_Transmit                             ...
%                             =   app.Activate;
%                         
%                     File_ID     =   fopen('Data.txt','a');
%                     fprintf(File_ID,[num2str(app.Input_Data), '\n']);
%                     fclose(File_ID);
%                     
%                     app.Input_Data                                 ...
%                             =   [];
%                 end
%                 
%                 File_ID     =   fopen('Data.txt','a');
%                 fprintf(File_ID,[num2str(app.Input_Data), '\n']);
%                 fclose(File_ID);
%                 
%             else
%                 
%                 app.Input_Data(end + 1)                            ...
%                             =   fread(app.Port, 1, 'uint8');
%                 
%                 if (app.Input_Data(end) == 0)
% 
%                  	File_ID     =   fopen('Data.txt','a');
%                     fprintf(File_ID,[num2str(app.Input_Data(end)), '\n']);
%                     fclose(File_ID);
%                     
%                     app.Signal                                     ...
%                             =	typecast(COBS_Decod(               ...
%                                             app.Input_Data),       ...
%                                          'single');
% 
%                     app.Y_Data                                     ...
%                             = 	Transform_Read_Data(app,           ...
%                                                     app.Y_Data,    ...
%                                                     app.Signal);
% 
%                     Data_Collector_And_Plottin(app);
%                     
%                     app.Input_Data                                 ...
%                             =   [];
%                 end
%                 
%                	File_ID     =   fopen('Data.txt','a');
%                 fprintf(File_ID,[num2str(app.Input_Data(end)), '\n']);
%                 fclose(File_ID);
%                 
%             end
%             
%         end
        
        %%  Read RS232 for find zero byte
        function Read_RS232_or_Find_Zero_byte(app)
            
            if (app.Frame_Transmit == app.DeActivate)
                
                app.Input_Data                                     ...
                      	=   fread(app.Port, 512, 'uint8');
                        
                if (app.Input_Data(1) == 0)
                    
                    app.Frame_Transmit                             ...
                            =   app.Activate;
                        
                    File_ID     =   fopen('Data.txt','a');
                    fprintf(File_ID,[num2str(app.Input_Data), '\n']);
                    fclose(File_ID);
                    
                    app.Input_Data                                 ...
                            =   [];
                end
                
                File_ID     =   fopen('Data.txt','a');
                fprintf(File_ID,[num2str(app.Input_Data), '\n']);
                fclose(File_ID);
                
            else
                
                app.Input_Data(end + 1)                            ...
                            =   fread(app.Port, 1, 'uint8');
                
                if (app.Input_Data(end) == 0)

                 	File_ID     =   fopen('Data.txt','a');
                    fprintf(File_ID,[num2str(app.Input_Data(end)), '\n']);
                    fclose(File_ID);
                    
                    app.Signal                                     ...
                            =	typecast(COBS_Decod(               ...
                                            app.Input_Data),       ...
                                         'single');

                    app.Y_Data                                     ...
                            = 	Transform_Read_Data(app,           ...
                                                    app.Y_Data,    ...
                                                    app.Signal);

                    Data_Collector_And_Plottin(app);
                    
                    app.Input_Data                                 ...
                            =   [];
                end
                
               	File_ID     =   fopen('Data.txt','a');
                fprintf(File_ID,[num2str(app.Input_Data(end)), '\n']);
                fclose(File_ID);
                
            end
            
        end
    end

    %% Programm methods that correspond to Collback 
    methods (Access = private)

        %% Code that executes after component creation
        function Startup_Collback(app)
            
            close all
            delete(instrfindall);
                        
            Port_List = instrhwinfo('serial');
            
            if (~isempty(Port_List.AvailableSerialPorts))
                
                app.SerialPortDropDown.Items                       ...
                            =     Port_List.AvailableSerialPorts;
                                                                                                                     
            end
            
        end

        %% Button pushed function: MeasurementButton
        function Mesuarments_Collback(app, event)

            Red           	=   [1, 0, 0];
            Green           =   [0, 1, 0];

            if (event.Source.BackgroundColor == Red)

                app.Port    =	serial(app.SerialPortDropDown.Value);

                event.Source.BackgroundColor                       ...
                           	= 	Green;

                app.Length_Input_Buffer                            ...
                            =   str2double(app.CellGetNumber.Value);

                app.Port.BaudRate                                  ...
                            =	str2double(app.BitRateKnob.Value);
                app.Port.BytesAvailableFcnCount                    ...
                          	=  	app.Length_Input_Buffer;
                app.Port.BytesAvailableFcnMode                     ...
                          	=	'byte';
                app.Port.BytesAvailableFcn                     	   ...
                          	=	@(~,~)Read_Data(app);

                app.SerialPortDropDown.BackgroundColor             ...
                           	=	Green;


                Time_Line  	=	(0: double(app.Length_Signal) - 1) ...
                                        ./ double(app.Length_Signal);                         
                app.Line_Handle                                    ...
                            = 	line(app.Axes, Time_Line,          ...
                                               zeros(1,            ...
                                                     length(       ...
                                                        Time_Line  ...
                                                           )       ...
                                                    )              ...
                                    );
                set(app.Line_Handle, 'Color',       'r',           ...
                                     'LineWidth',   1.25,          ...
                                     'LineStyle',   '--')
                axis(app.Axes, [0, 1, -12, 12])
                app.Axes.XTick                                     ...
                            =   0:0.1:1;
                app.Axes.YTick                                     ...
                            =   -12:3:12;
                
                app.Frame_Transmit                                 ...
                            =   app.DeActivate;
                        
                fopen(app.Port);
                
%                 Read_RS232_or_Find_Zero_byte(app)

            else

                event.Source.BackgroundColor                       ...
                           	= 	Red;

                stopasync(app.Port);
                fclose(app.Port);

                app.SerialPortDropDown.BackgroundColor             ...
                        	=  	Red;

            end
        end
        
    end

    %% App initialization and construction
    methods (Access = private)

        % Create Figure_Handler and components
        function createComponents(app)

            %% Create Figure_Handler
            
            app.Figure_Handler                                     ...
                            = uifigure('Name',                     ...
                                        'Receive_Data_And_Ploting',...
                                       'Menu', 'No',               ...
                                       'Color', app.Color_Background);
                                   
            Root_Obj        =   findobj(0, 'Type', 'root');
            Monitor_Size    =   get(Root_Obj, 'ScreenSize');
            app.Figure_Handler.Position                            ...
                            =   Monitor_Size;
            setAutoResize(app, app.Figure_Handler, true)

            %% Create Axes
            
            app.Axes        =   uiaxes(app.Figure_Handler);
            app.Axes.Units  =   'Pixel';
            app.Axes.Box    =   'on';
            app.Axes.Position                                      ...
                            =   [Monitor_Size(1),                  ...
                                 Monitor_Size(2) - 18 +            ...
                                    int32(Monitor_Size(4) * 1 / 3),...   
                                 int32(Monitor_Size(3) * 3 / 4),   ...
                                 int32(Monitor_Size(4) * 2 / 3)];
            xlabel(app.Axes, 'Time, t, sec');
            ylabel(app.Axes, 'Acceleration, g, m_{/_{sec^2}}');
            app.Axes.FontName                                      ...
                            =   'RomanC';
            app.Axes.FontSize                                      ...
                            =   14;
            app.Axes.FontWeight                                    ...
                            =   'bold';
            app.Axes.Box   	=   'on';
            app.Axes.Color 	=   app.Color_Background;
            app.Axes.XGrid  =   'on';
            app.Axes.YGrid 	=   'on';

            %%  Create ConfigurationPanel
            
            app.ConfigurationPanel                                 ...
                            =   uipanel(app.Figure_Handler);
            app.ConfigurationPanel.TitlePosition                   ...
                            =   'centertop';
            app.ConfigurationPanel.Title                           ...
                            =   'Configuration Panel';
            app.ConfigurationPanel.BackgroundColor                 ...
                            =   app.Color_Background;
            app.ConfigurationPanel.FontName                        ...
                            =   'RomanC';
            app.ConfigurationPanel.FontWeight                      ...
                            =   'bold';
            app.ConfigurationPanel.FontSize                        ...
                            =   14;
            Panel_Size      =   [Monitor_Size(1:2),                ...
                                 int32(Monitor_Size(3) * 1 / 3),   ...
                                 int32(Monitor_Size(4) * 1 / 3) - 18];
            app.ConfigurationPanel.Position                        ...
                            =   Panel_Size;
        
            Panel_Size(4)   =   Panel_Size(4) - 23;

            %%  Create SerialPortDropDown
        
            Label_Higth    	=   20;
            DropDown_Higth 	=   Panel_Size(4) / 2 - Label_Higth;
        
            app.SerialPortDropDown                                 ...
                            =   uidropdown(app.ConfigurationPanel);
            app.SerialPortDropDown.Items                           ...
                            =   {'No Ports'};
            app.SerialPortDropDown.FontName                        ...
                            =   'RomanC';
            app.SerialPortDropDown.FontSize                        ...
                            =   14;
            app.SerialPortDropDown.FontWeight                      ...
                            =   'bold';
            app.SerialPortDropDown.BackgroundColor                 ...
                            =   [1 0 0];
            app.SerialPortDropDown.Position                        ...
                            =   [3, Panel_Size(4) / 2 + 3,         ...
                                 Panel_Size(3) * 1 / 3 - 3,        ...
                                 DropDown_Higth - 3                ...
                                ];
            app.SerialPortDropDown.Value                           ...
                            =   'No Ports';

            %%  Create SerialPortDropDownLabel
            app.SerialPortDropDownLabel                            ...
                            =   uilabel(app.ConfigurationPanel);
            app.SerialPortDropDownLabel.HorizontalAlignment        ...
                            =   'center';
            app.SerialPortDropDownLabel.VerticalAlignment          ...
                            =   'center';
            app.SerialPortDropDownLabel.FontName                   ...
                            =   'RomanC';
            app.SerialPortDropDownLabel.FontSize                   ...
                            =   14;
            app.SerialPortDropDownLabel.FontWeight                 ...
                            =   'bold';
            app.SerialPortDropDownLabel.Position                   ...
                            =   [3,                                ...
                                 Panel_Size(4) - Label_Higth + 3,  ...
                                 Panel_Size(3) * 1 / 3 - 6,        ...
                                 Label_Higth - 3                   ...
                                ];
            app.SerialPortDropDownLabel.Text                       ...
                            =   'Serial Port';

            %%  Create BitRateKnobLabel
            app.BitRateKnobLabel                                   ...
                            =   uilabel(app.ConfigurationPanel);
            app.BitRateKnobLabel.HorizontalAlignment               ...
                            =   'center';
            app.BitRateKnobLabel.VerticalAlignment                 ...
                            =   'center';
            app.BitRateKnobLabel.FontName                          ...
                            =   'RomanC';
            app.BitRateKnobLabel.FontSize                          ...
                            =   14;
            app.BitRateKnobLabel.FontWeight                        ...
                            =   'bold';
            app.BitRateKnobLabel.Position                          ...
                            =   [Panel_Size(3) * 1 / 3 + 3,        ...
                                 Panel_Size(4) - Label_Higth + 3,  ...
                                 Panel_Size(3) * 2 / 3 - 3,        ...
                                 Label_Higth - 3];
            app.BitRateKnobLabel.Text                              ... 
                            =   'Bit Rate';

            %%  Create BitRateKnob
        
            Width_Knob      =   (Panel_Size(4) + Label_Higth + 5) / 2;
            Width_Knob      =   Width_Knob - 10;
        
            app.BitRateKnob =   uiknob(app.ConfigurationPanel,     ...
                                       'discrete');
            app.BitRateKnob.Items                                  ...
                            =   {'7200',                           ...
                                 '14400',                          ...
                                 '28800',                          ...
                                 '57600',                          ...
                                 '115200'};
            app.BitRateKnob.FontName                               ...
                            =   'RomanC';
            app.BitRateKnob.FontSize                               ...
                            =   14;
            app.BitRateKnob.FontWeight                             ...
                            =   'bold';
            app.BitRateKnob.Position                               ...
                            =   [- 0.5 * Width_Knob +              ...
                                         Panel_Size(3) * 4 / 6 + 3,...
                                 5  +  2    * Label_Higth          ...
                                 Width_Knob,                       ...
                                 Width_Knob];
            app.BitRateKnob.Value                                  ...
                            =   '7200';

            %%  Create CellGetNumber
            app.CellGetNumber                                      ...
                            =   uieditfield(app.ConfigurationPanel,...
                                            'text');
            app.CellGetNumber.HorizontalAlignment                  ...
                            =   'center';
            app.CellGetNumber.FontName                             ...
                            =   'RomanC';
            app.CellGetNumber.FontSize                             ...
                            =   14;
            app.CellGetNumber.FontWeight                           ...
                            =   'bold';
            app.CellGetNumber.BackgroundColor                      ...
                            =   app.Color_Background;
            app.CellGetNumber.Position                             ...
                            =   [Panel_Size(3) * 1 / 3 + 3,        ...
                                 5,                                ...
                                 Panel_Size(3) * 2 / 3 - 6,        ...
                                 2 * Label_Higth - 3];
            app.CellGetNumber.Value                                    ...
                            =   'Number Discrete';
        
            %%  Create MeasurementButton
            app.MeasurementButton                                      ...
                            =   uibutton(app.ConfigurationPanel,   ...
                                         'push');
            app.MeasurementButton.ButtonPushedFcn                      ...
                            =   @(source, event)                   ...
                                 Mesuarments_Collback(app, event);
            app.MeasurementButton.BackgroundColor                      ...
                            =   [1 0 0];
            app.MeasurementButton.FontName                             ...
                            =   'RomanC';
            app.MeasurementButton.FontSize                             ...
                            =   14;
            app.MeasurementButton.FontWeight                           ...
                            =   'bold';
            app.MeasurementButton.Position                             ...
                            =   [3, 5, Panel_Size(3) * 1 / 3 - 3,  ...
                                 Panel_Size(4) / 2 - 3];
            app.MeasurementButton.Text                                 ...
                            =   'Measurement';
        end
    end

    methods (Access = public)

        %% Construct app
        function app = Receive_Data_And_Ploting()

            % Create and configure components
            createComponents(app)

            % Register the app with App Designer
            registerApp(app, app.Figure_Handler)

            % Execute the startup function
            runStartupFcn(app, @Startup_Collback)

            if nargout == 0
                clear app
            end
        end

        %% Code that executes before app deletion
        function delete(app)

            % Delete UIFigure when app is deleted
            delete(app.Figure_Handler)
        end
    end
end


CREATE PROCEDURE [dbo].[ONLINE_NOTICE]
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @onlineCount INT;
    DECLARE @lightCount INT;
    DECLARE @furyCount INT;
    DECLARE @lightPercent INT;
    DECLARE @furyPercent INT;
    DECLARE @command NVARCHAR(400);

    -- job counts
    DECLARE @fighter INT, @defender INT, @ranger INT, @archer INT, @mage INT, @priest INT; -- Light
    DECLARE @warrior INT, @guardian INT, @assassin INT, @hunter INT, @pagan INT, @oracle INT; -- Fury

    WAITFOR DELAY '00:00:10'; 

    -- bilang ng online
    SELECT @onlineCount = COUNT(*)
    FROM PS_GameData.dbo.Chars
    WHERE LoginStatus = 1;

    -- bilang ng Light at Fury
    SELECT 
        @lightCount = SUM(CASE WHEN u.Country = 0 THEN 1 ELSE 0 END),
        @furyCount  = SUM(CASE WHEN u.Country = 1 THEN 1 ELSE 0 END)
    FROM PS_GameData.dbo.Chars c
    INNER JOIN PS_GameData.dbo.UserMaxGrow u
        ON c.UserUID = u.UserUID
    WHERE c.LoginStatus = 1;

    -- bilang ng bawat job (Light vs Fury)
    SELECT 
        @fighter  = SUM(CASE WHEN c.Job = 0 AND u.Country = 0 THEN 1 ELSE 0 END),
        @defender = SUM(CASE WHEN c.Job = 1 AND u.Country = 0 THEN 1 ELSE 0 END),
        @ranger   = SUM(CASE WHEN c.Job = 2 AND u.Country = 0 THEN 1 ELSE 0 END),
        @archer   = SUM(CASE WHEN c.Job = 3 AND u.Country = 0 THEN 1 ELSE 0 END),
        @mage     = SUM(CASE WHEN c.Job = 4 AND u.Country = 0 THEN 1 ELSE 0 END),
        @priest   = SUM(CASE WHEN c.Job = 5 AND u.Country = 0 THEN 1 ELSE 0 END),

        @warrior  = SUM(CASE WHEN c.Job = 0 AND u.Country = 1 THEN 1 ELSE 0 END),
        @guardian = SUM(CASE WHEN c.Job = 1 AND u.Country = 1 THEN 1 ELSE 0 END),
        @assassin = SUM(CASE WHEN c.Job = 2 AND u.Country = 1 THEN 1 ELSE 0 END),
        @hunter   = SUM(CASE WHEN c.Job = 3 AND u.Country = 1 THEN 1 ELSE 0 END),
        @pagan    = SUM(CASE WHEN c.Job = 4 AND u.Country = 1 THEN 1 ELSE 0 END),
        @oracle   = SUM(CASE WHEN c.Job = 5 AND u.Country = 1 THEN 1 ELSE 0 END)
    FROM PS_GameData.dbo.Chars c
    INNER JOIN PS_GameData.dbo.UserMaxGrow u
        ON c.UserUID = u.UserUID
    WHERE c.LoginStatus = 1;

    IF @onlineCount > 0
    BEGIN
        SET @lightPercent = (@lightCount * 100) / @onlineCount;
        SET @furyPercent  = (@furyCount * 100) / @onlineCount;

        -- IMPORTANT: eksaktong order ng values para tumugma sa sscanf
        SET @command = '/nt [ONLINE_NOTICE] ' 
                       + CAST(@onlineCount AS NVARCHAR(10)) + ' '   -- total
                       + CAST(@lightCount AS NVARCHAR(10)) + ' '    -- lightCount
                       + CAST(@lightPercent AS NVARCHAR(10)) + ' '  -- lightPercent
                       + CAST(@furyCount AS NVARCHAR(10)) + ' '     -- furyCount
                       + CAST(@furyPercent AS NVARCHAR(10)) + ' '   -- furyPercent
                       + CAST(@fighter AS NVARCHAR(10)) + ' '       -- Light jobs
                       + CAST(@defender AS NVARCHAR(10)) + ' '
                       + CAST(@ranger AS NVARCHAR(10)) + ' '
                       + CAST(@archer AS NVARCHAR(10)) + ' '
                       + CAST(@mage AS NVARCHAR(10)) + ' '
                       + CAST(@priest AS NVARCHAR(10)) + ' '
                       + CAST(@warrior AS NVARCHAR(10)) + ' '       -- Fury jobs
                       + CAST(@guardian AS NVARCHAR(10)) + ' '
                       + CAST(@assassin AS NVARCHAR(10)) + ' '
                       + CAST(@hunter AS NVARCHAR(10)) + ' '
                       + CAST(@pagan AS NVARCHAR(10)) + ' '
                       + CAST(@oracle AS NVARCHAR(10));

        EXEC securetech.dbo.RunServiceCommand 
            @Service = N'ps_game', 
            @CommandText = @command;
    END
    ELSE
    BEGIN
        SET @lightPercent = 0;
        SET @furyPercent  = 0;
    END

    -- Always return a row para sa C++ app
    SELECT 
        @onlineCount AS OnlineCount,
        @lightCount AS LightCount,
        @lightPercent AS LightPercent,
        @furyCount AS FuryCount,
        @furyPercent AS FuryPercent,
        @fighter AS Fighter,
        @defender AS Defender,
        @ranger AS Ranger,
        @archer AS Archer,
        @mage AS Mage,
        @priest AS Priest,
        @warrior AS Warrior,
        @guardian AS Guardian,
        @assassin AS Assassin,
        @hunter AS Hunter,
        @pagan AS Pagan,
        @oracle AS Oracle,
        ISNULL(@command, '') AS CommandString;

    -- dagdag: listahan ng mga CharName na online
    SELECT 
        c.CharName,
        c.UserUID,
        u.Country
    FROM PS_GameData.dbo.Chars c
    INNER JOIN PS_GameData.dbo.UserMaxGrow u
        ON c.UserUID = u.UserUID
    WHERE c.LoginStatus = 1;
END;

USE [PS_GameLog]
GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[PvP_Log](
	[RowID] [int] IDENTITY(1,1) NOT NULL,
	[UserID] [varchar](18) NULL,
	[UserUID] [int] NULL,
	[CharID] [int] NULL,
	[CharName] [varchar](50) NULL,
	[MapID] [smallint] NULL,
	[PosX] [float] NULL,
	[PosY] [float] NULL,
	[PosZ] [float] NULL,
	[ActionTime] [datetime2](3) NULL,
	[ActionType] [tinyint] NULL,
	[UserIP] [varchar](50) NULL,
PRIMARY KEY CLUSTERED 
(
	[RowID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO



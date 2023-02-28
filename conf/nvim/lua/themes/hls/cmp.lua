return function(colors)
  return {
    CmpItemKindClass = { link = "Type" },
    CmpItemKindConstant = { link = "Constant" },
    CmpItemKindConstructor = { link = "TSConstructor" },
    CmpItemKindFunction = { link = "Function" },
    CmpItemKindKeyword = { link = "Keyword" },
    CmpItemKindOperator = { link = "Operator" },
    CmpItemKindProperty = { link = "TSProperty" },
    CmpItemKindStruct = { link = "Structure" },
    CmpItemKindTypeParameter = { link = "TSParameter" },
    CmpItemKindUnit = { link = "Number" },
    CmpItemKindValue = { link = "String" },
    CmpItemKindVariable = { link = "Identifier" },
    CmpDocumentationBorder = { fg = colors.foreground, bg = colors.background },
    CmpItemAbbr = { fg = colors.foreground, bg = colors.background },
    CmpItemAbbrDeprecated = { fg = colors.color2, bg = colors.background },
    CmpItemAbbrMatch = { fg = colors.color7, bg = colors.background },
    CmpItemAbbrMatchFuzzy = { fg = colors.color7, bg = colors.background },
    CmpItemKind = { fg = colors.color12, bg = colors.background },
    CmpItemMenu = { fg = colors.color2, bg = colors.background },
  }
end

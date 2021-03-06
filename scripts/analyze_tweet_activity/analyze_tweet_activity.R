## Analyze tweet impressions
## usage : Rscript analyze_tweet_activity.R infile outfile
## infile : Name of exported tweet analytics CSV file (input)
## outfile : Name of PNG image file (output)

library(Cairo)
library(data.table)
library(fitdistrplus)
library(functional)

default_infilename <- 'data/in.csv'   # input data filename
default_outfilename <- 'data/out.png' # output image filename

## Range fit by analyze_tweet_activity_range.py
low_impression_threshold  <- 82   # less than half of followers
high_impression_threshold <- 294  # exp(6)

## Assume tweets are posted by a bot when they are posted at
## even-hour, 00-05 minute UTC.
is_bot <- function(time_str) {
    grepl(' \\d[02468]:\\d[0-5] ', time_str)
}

## Return whether each column contains a word
contains_word <- function(word, col) {
    ## Assume the word contains no escapes
    grepl(word, col)
}

exp_linear_string <- function(x) {
    sprintf('%3.0f', floor(exp(x)))
}

draw_exp_linear_labels <- function(data) {
    ticks <- seq(min(data), max(data), (max(data) - min(data)) / 10)
    labels <- lapply(ticks, exp_linear_string)
    axis(1, at=unlist(ticks), labels=labels, col.axis='orchid', las=2, cex=0.4)
}

## Draws P-P plot and histogram
## X-axis: impression (linear), Y-axis: frequency of tweets (linear or log)
draw_pp_hist <- function(data, model, title, xlabel, show_estimation, show_labal) {
    ylabel <- 'Frequency of tweets'
    ## Estimates a distribution under the 'model'
    fd <- fitdist(data, model)
    ## P-P plot
    ppcomp(fd, legendtext='tweet', fitcol='violetred')

    if (show_estimation == TRUE) {
        hist(data, xlab=xlabel, ylab=ylabel, main=title, prob=TRUE, xaxt='n', yaxt='n')
        # Overwrites an estimatsion under the 'model'.
        # The first parameter for curve must have name 'x'.
        x <- data
        curve(dnorm(x, mean=fd$estimate[1], sd=fd$estimate[2]),
              xlab=xlabel, col='royalblue', lwd=2, add=TRUE)
    } else {
        hist(data, xlab=xlabel, ylab=ylabel, main=title, prob=TRUE, xaxt='n', yaxt='n')
    }

    if (show_labal == TRUE) {
        draw_exp_linear_labels(data)
    }
}

## Draws P-P plot and histogram (tweet impressions are plotted on a log scale)
draw_pp_loghist <- function(data, title, xlabel) {
    hist.data <- hist(data, breaks=10, xlab='ln(Impressions)', ylab='Frequency of tweets',
                     main=title, prob=TRUE, xaxt='n', yaxt='n')

    ## Draws a chart in log-log scales to confirm whether data are
    ## distributed under the power law.
    hist_log.data <- hist(data, breaks=10, plot=FALSE)
    hist_log.data$counts <- log(hist_log.data$counts + 1)
    plot(hist_log.data, xaxt='n', yaxt='n',
         main=title, xlab='ln(Impressions)', ylab='ln(Frequency of tweets)')

    ## Overwrites a regression line
    m <- lm(hist_log.data$counts ~ hist_log.data$mids)
    abline(m, col='royalblue', lwd=2)
    draw_exp_linear_labels(data)
}

## Draws all charts
draw_all_pp_hist <- function(impressions, low_impressions, high_impressions, fontsize) {
    par(mfcol=c(2, 5), ps=fontsize)

    draw_pp_hist(impressions, 'norm', 'Impressions', 'Impressions', FALSE, FALSE)
    draw_pp_hist(low_impressions, 'norm', 'Except high impressions', 'Impressions', TRUE, FALSE)

    draw_pp_hist(log(impressions), 'norm', 'Impressions', 'ln(Impressions)', FALSE, TRUE)
    draw_pp_hist(log(low_impressions), 'norm', 'Except high impressions', 'ln(Impressions)', TRUE, TRUE)

    draw_pp_loghist(log(high_impressions), 'High impressions', 'ln(Impressions)')
}

## Input and output filenames
args <- commandArgs(trailingOnly=TRUE)
infilename <- ifelse(length(args) >= 1, args[1], default_infilename)
outfilename <- ifelse(length(args) >= 2, args[2], default_outfilename)

## Loads a tweet activity CSV file (its titles must be swapped to English)
raw_data <- data.table(read.csv(file=file.path(infilename),
                                header=TRUE, fileEncoding='utf-8',
                                stringsAsFactors=FALSE))

## Excludes N/A data
data <- raw_data[complete.cases(raw_data),]

## Sets whether each tweets are possibly posted by the bot
data$bot <- unlist(lapply(data$time, is_bot))

## Classify hashtags
data$Hashtag <- factor(data$Hashtag)
#data$newgame <- unlist(lapply(data$Tweet.text, Curry(contains_word, word='#newgame')))

## Cut tweets with low impressions that suggest they are missed in observations
data_no_low <- data[data$impression >= low_impression_threshold]
## Separate low impressions from high impressions which have different distributions
data_middle <- data_no_low[data_no_low$impressions < high_impression_threshold]
data_high <- data[data$impressions >= high_impression_threshold]

## Excludes manual posts
## Cut tweets with low impressions that suggest they are missed in observations
no_low_impressions <- data_no_low[data_no_low$bot == TRUE]$impressions
middle_impressions <- data_middle[data_middle$bot == TRUE]$impressions
high_impressions <- data_high[data_high$bot == TRUE]$impressions

## Draws on screen and writes to a PNG file
draw_all_pp_hist(no_low_impressions, middle_impressions, high_impressions, 16)
png(filename=outfilename, width=1600, height=800)
draw_all_pp_hist(no_low_impressions, middle_impressions, high_impressions, 16)
dev.off()

## Includes manual posts
data_no_high <- data[data$impressions < high_impression_threshold]
nrow(data_no_high)
boxplot(log(data_no_high$impressions))
data_no_high.glm <- glm(formula = impressions ~ retweets + replies + likes + bot, data_no_high, family=gaussian(log))
data_no_high.step <- stepAIC(data_no_high.glm, trace = FALSE)
summary(data_no_high.step)

data_middle <- data_no_high[data_no_high$impressions >= low_impression_threshold]
nrow(data_middle)
boxplot(log(data_middle$impressions))
data_middle.glm <- glm(formula = impressions ~ retweets + replies + likes + bot, data_middle, family=gaussian(log))
data_middle.step <- stepAIC(data_middle.glm, trace = FALSE)
summary(data_middle.step)

nrow(data)
boxplot(log(data$impressions))
data.glm <- glm(formula = impressions ~ retweets + replies + likes + bot, data, family=gaussian(log))
data.step <- stepAIC(data.glm, trace = FALSE)
summary(data.step)

data_hashtag <- data_middle[data_middle$bot == TRUE]
#data_hashtag <- data[data$bot == TRUE]
boxplot(log(impressions) ~ Hashtag, data=data_hashtag,
        title='Middle', xlab='Hashtag', ylab='log(tweet impression)')
data_hashtag_lm <- lm(formula = log(impressions) ~ Hashtag, data_hashtag)
summary(data_hashtag_lm)

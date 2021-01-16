(define (problem ijrr_5)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street14 street29 - street
		building4 building25 building29 - building
	)
	(:init
		(atLocation survivor0 street14)
		(atLocation survivor1 building25)
		(onFire building4)
		(onFire building29)
		(underRubble street29)
		(underRubble building4)
		(needsRepair building4)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street29))
		(not (needsRepair building4))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
